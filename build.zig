const std = @import("std");
const mem = std.mem;
const fs = std.fs;

const GraphicsPlaygroundOptions = struct {
    target: std.Build.ResolvedTarget,
    optimize: std.builtin.OptimizeMode,
    enable_asan: bool = false,
    warnings_as_errors: bool = false,

    pub fn cppFlags(self: *const GraphicsPlaygroundOptions) []const []const u8 {
        if (self.warnings_as_errors) {
            return &[_][]const u8{ "-std=c++20", "-Wall", "-Wextra", "-Wpedantic", "-Wno-deprecated-volatile", "-Werror" };
        } else {
            return &[_][]const u8{ "-std=c++20", "-Wall", "-Wextra", "-Wpedantic", "-Wno-deprecated-volatile" };
        }
    }
};

const Library = struct {
    lib: *std.Build.Step.Compile,
    include_paths: []const std.Build.LazyPath,
    deps: []const *const Library,

    pub fn init(b: *std.Build, lib: *std.Build.Step.Compile, include_paths: []const std.Build.LazyPath, deps: []const *const Library) Library {
        const include_paths_copy = b.allocator.alloc(std.Build.LazyPath, include_paths.len) catch @panic("OOM");
        for (include_paths_copy, include_paths) |*dest, source| {
            dest.* = source;
        }
        const deps_copy = b.allocator.alloc(*const Library, deps.len) catch @panic("OOM");
        for (deps_copy, deps) |*dest, source| {
            dest.* = source;
        }
        return Library{ .lib = lib, .include_paths = include_paths_copy, .deps = deps_copy };
    }
};

pub fn build(b: *std.Build) !void {
    const options = GraphicsPlaygroundOptions{
        .target = b.standardTargetOptions(.{}),
        .optimize = b.standardOptimizeOption(.{}),
        .enable_asan = b.option(bool, "enable-asan", "Enable address sanitizer") orelse false,
        .warnings_as_errors = b.option(bool, "warnings-as-errors", "Treat warnings as errors") orelse false,
    };

    const glfw = b.dependency("glfw", .{ .target = options.target, .optimize = options.optimize });
    const curl_dependency = b.dependency("curl", .{
        .target = options.target,
        .optimize = options.optimize,
        .libpsl = false,
        .libssh2 = false,
        .libidn2 = false,
        .nghttp2 = false,
        .@"disable-ldap" = true,
    });

    const glad_lib = try createLibraryGlad(b, options);
    const glfw_lib = Library.init(b, glfw.artifact("glfw"), &.{}, &.{});
    const imgui_lib = try createLibraryImGUI(b, options, &.{ &glad_lib, &glfw_lib });
    const opengl_lib = try createLibraryOpenGL(b, options, &.{&glad_lib});
    const core_lib = try createLibraryCore(b, options, &.{ &opengl_lib, &imgui_lib });

    const zlib_lib = try createLibraryZlib(b, options);
    const jpeg_lib = try createLibraryJpeg(b, options);
    const png_lib = try createLibraryPng(b, options, &.{&zlib_lib});
    const curl_lib = Library.init(b, @import("curl").artifact(curl_dependency, .lib), &.{}, &.{});

    var libraries = std.StringHashMap(Library).init(b.allocator);
    defer libraries.deinit();

    const fast_noise_lib = try createLibraryFastNoise(b, options, &.{&imgui_lib});
    const image_lib = try createLibraryImage(b, options, &.{ &opengl_lib, &jpeg_lib, &png_lib, &zlib_lib });
    try libraries.put("fast_noise", fast_noise_lib);
    try libraries.put("marching_cubes_lib", try createLibraryMarchingCubes(b, options, &.{&fast_noise_lib}));
    try libraries.put("image", image_lib);
    try libraries.put("model", try createLibraryModel(b, options, &.{ &core_lib, &image_lib }));
    try libraries.put("zlib", zlib_lib);
    try libraries.put("zip", try createLibraryZip(b, options, &.{&zlib_lib}));
    try libraries.put("curl", curl_lib);

    try createScenes(b, options, &.{ &core_lib, &glad_lib, &opengl_lib, &imgui_lib }, libraries);
}

fn compileShader(b: *std.Build, scene_name: []const u8, shader_path: []const u8, output_dir: []const u8) ![]const u8 {
    const shader_file = try fs.cwd().openFile(shader_path, .{});
    defer shader_file.close();

    const stream_buf =  [_]u8{0} ** 4096;
    var in_stream = shader_file.reader(stream_buf);

    var shader_lines = std.ArrayList([]const u8).init(b.allocator);
    defer {
        for (shader_lines.items) |line| {
            b.allocator.free(line);
        }
        shader_lines.deinit();
    }

    var buf: [4096]u8 = undefined;
    while (try in_stream.readUntilDelimiterOrEof(&buf, '\n')) |line| {
        const duped_line = try b.allocator.dupe(u8, line);
        try shader_lines.append(duped_line);
    }

    const base_name = std.fs.path.stem(shader_path);
    const shader_name = try std.fmt.allocPrint(b.allocator, "{s}_{s}", .{ scene_name, base_name });
    defer b.allocator.free(shader_name);

    const output_file_path = b.pathJoin(&.{ output_dir, b.fmt("{s}.cpp", .{shader_name}) });
    const output_file = try fs.cwd().createFile(output_file_path, .{});
    defer output_file.close();

    std.debug.print("\tCompiling shader: {s} -> {s}\n", .{ shader_path, output_file_path });

    var writer = output_file.writer();
    try writer.print("extern \"C\" {{\n", .{});
    try writer.print("unsigned int {s}_len = {};\n", .{ shader_name, shader_lines.items.len });
    try writer.print("const char *{s}[] = {{\n", .{shader_name});

    for (shader_lines.items) |line| {
        const escaped_line = try std.mem.replaceOwned(u8, b.allocator, line, "\"", "\\\"");
        defer b.allocator.free(escaped_line);
        try writer.print("      \"{s}\\n\",\n", .{escaped_line});
    }

    try writer.print("}};\n", .{});

    try writer.print("int {s}_line_lens[{}] = {{", .{ shader_name, shader_lines.items.len });
    for (shader_lines.items, 0..) |line, line_idx| {
        if (line_idx > 0) {
            try writer.print(", ", .{});
        }
        try writer.print("{}", .{line.len + 1}); // +1 for the newline
    }
    try writer.print("}};\n", .{});

    try writer.print("const char* {s}_file_path = \"{s}\";\n", .{ shader_name, b.pathJoin(&.{ b.build_root.path.?, shader_path }) });
    try writer.print("}}\n", .{});

    return output_file_path;
}

fn compileShaders(b: *std.Build, scene_name: []const u8, scene_path: []const u8) !std.ArrayList([]const u8) {
    var dir = try std.fs.cwd().openDir(scene_path, .{ .iterate = true });
    defer dir.close();

    var walker = try dir.walk(b.allocator);
    defer walker.deinit();

    var shader_paths = std.ArrayList([]const u8).empty;
    while (try walker.next()) |entry| {
        if (entry.kind != .file) continue;

        const ext = std.fs.path.extension(entry.basename);
        if (!std.mem.eql(u8, ext, ".glsl")) continue;

        const shader_path = b.pathJoin(&.{ scene_path, entry.path });
        defer b.allocator.free(shader_path);

        const output_dir = b.pathJoin(&.{ ".zig-cache", "generated", "shaders", scene_name });
        defer b.allocator.free(output_dir);

        // Create output directories if they don't exist
        fs.cwd().makePath(output_dir) catch |err| {
            if (err != error.PathAlreadyExists) {
                return err;
            }
        };

        const source_file_path = try compileShader(b, scene_name, shader_path, output_dir);
        try shader_paths.append(b.allocator,source_file_path);
    }

    return shader_paths;
}

fn linkLibrary(step: *std.Build.Step.Compile, lib: *const Library) void {
    step.linkLibrary(lib.lib);
    for (lib.include_paths) |include_path| {
        step.addIncludePath(include_path);
    }
    for (lib.deps) |dep| {
        // TODO handle circular dependencies
        linkLibrary(step, dep);
    }
}

fn createLibraryGlad(b: *std.Build, options: GraphicsPlaygroundOptions) !Library {
    const glad_lib = b.addLibrary(.{
        .name = "glad",
        .linkage = std.builtin.LinkMode.static,
        .root_module = b.createModule(.{
            .target = options.target,
            .optimize = options.optimize,
            .link_libc = true,
        }),
    });
    const include_path = b.path("src/libs/glad/include");
    glad_lib.addIncludePath(include_path);
    const sources = try findSources(b, "src/libs/glad/src", &.{});
    glad_lib.addCSourceFiles(.{ .files = sources });
    return Library.init(b, glad_lib, &.{include_path}, &.{});
}

fn createLibraryJpeg(b: *std.Build, options: GraphicsPlaygroundOptions) !Library {
    const jpeg_lib = b.addLibrary(.{
        .name = "jpeg",
        .root_module = b.createModule(.{
            .target = options.target,
            .optimize = options.optimize,
            .link_libc = true,
        }),
    });
    const include_path = b.path("build/_deps/libjpeg-src");
    jpeg_lib.addIncludePath(include_path);
    jpeg_lib.addCSourceFiles(.{
        .files = &.{
            "build/_deps/libjpeg-src/jmemnobs.c",
            "build/_deps/libjpeg-src/jaricom.c",
            "build/_deps/libjpeg-src/jcapimin.c",
            "build/_deps/libjpeg-src/jcapistd.c",
            "build/_deps/libjpeg-src/jcarith.c",
            "build/_deps/libjpeg-src/jccoefct.c",
            "build/_deps/libjpeg-src/jccolor.c",
            "build/_deps/libjpeg-src/jcdctmgr.c",
            "build/_deps/libjpeg-src/jchuff.c",
            "build/_deps/libjpeg-src/jcinit.c",
            "build/_deps/libjpeg-src/jcmainct.c",
            "build/_deps/libjpeg-src/jcmarker.c",
            "build/_deps/libjpeg-src/jcmaster.c",
            "build/_deps/libjpeg-src/jcomapi.c",
            "build/_deps/libjpeg-src/jcparam.c",
            "build/_deps/libjpeg-src/jcprepct.c",
            "build/_deps/libjpeg-src/jcsample.c",
            "build/_deps/libjpeg-src/jctrans.c",
            "build/_deps/libjpeg-src/jdapimin.c",
            "build/_deps/libjpeg-src/jdapistd.c",
            "build/_deps/libjpeg-src/jdarith.c",
            "build/_deps/libjpeg-src/jdatadst.c",
            "build/_deps/libjpeg-src/jdatasrc.c",
            "build/_deps/libjpeg-src/jdcoefct.c",
            "build/_deps/libjpeg-src/jdcolor.c",
            "build/_deps/libjpeg-src/jddctmgr.c",
            "build/_deps/libjpeg-src/jdhuff.c",
            "build/_deps/libjpeg-src/jdinput.c",
            "build/_deps/libjpeg-src/jdmainct.c",
            "build/_deps/libjpeg-src/jdmarker.c",
            "build/_deps/libjpeg-src/jdmaster.c",
            "build/_deps/libjpeg-src/jdmerge.c",
            "build/_deps/libjpeg-src/jdpostct.c",
            "build/_deps/libjpeg-src/jdsample.c",
            "build/_deps/libjpeg-src/jdtrans.c",
            "build/_deps/libjpeg-src/jerror.c",
            "build/_deps/libjpeg-src/jfdctflt.c",
            "build/_deps/libjpeg-src/jfdctfst.c",
            "build/_deps/libjpeg-src/jfdctint.c",
            "build/_deps/libjpeg-src/jidctflt.c",
            "build/_deps/libjpeg-src/jidctfst.c",
            "build/_deps/libjpeg-src/jidctint.c",
            "build/_deps/libjpeg-src/jquant1.c",
            "build/_deps/libjpeg-src/jquant2.c",
            "build/_deps/libjpeg-src/jutils.c",
            "build/_deps/libjpeg-src/jmemmgr.c",
        },
        .flags = &.{"-std=gnu90"},
    });
    const config_header = b.addConfigHeader(.{
        .style = .{ .cmake = b.path("build/_deps/libjpeg-src/jconfig.h.in") },
        .include_path = "jconfig.h",
    }, .{});
    jpeg_lib.addConfigHeader(config_header);
    var full_file_path = config_header.getOutput();
    full_file_path.generated.up = 1;
    return Library.init(b, jpeg_lib, &.{ include_path, full_file_path }, &.{});
}

fn createLibraryPng(b: *std.Build, options: GraphicsPlaygroundOptions, deps: []const *const Library) !Library {
    // TODO add optimizations for different architectures
    const png_lib = b.addLibrary(.{
        .name = "png",
        .root_module = b.createModule(.{
            .target = options.target,
            .optimize = options.optimize,
        }),
    });
    const png_set_jump = if (options.target.result.isWasiLibC()) "-D PNG_NO_SETJMP" else "";
    const include_path = b.path("build/_deps/libpng-src");
    png_lib.addIncludePath(include_path);
    png_lib.addCSourceFiles(.{
        .files = &.{
            "build/_deps/libpng-src/png.c",
            "build/_deps/libpng-src/pngerror.c",
            "build/_deps/libpng-src/pngget.c",
            "build/_deps/libpng-src/pngmem.c",
            "build/_deps/libpng-src/pngpread.c",
            "build/_deps/libpng-src/pngread.c",
            "build/_deps/libpng-src/pngrio.c",
            "build/_deps/libpng-src/pngrtran.c",
            "build/_deps/libpng-src/pngrutil.c",
            "build/_deps/libpng-src/pngset.c",
            "build/_deps/libpng-src/pngtrans.c",
            "build/_deps/libpng-src/pngwio.c",
            "build/_deps/libpng-src/pngwrite.c",
            "build/_deps/libpng-src/pngwtran.c",
            "build/_deps/libpng-src/pngwutil.c",
        },
        .flags = &.{ "-D PNG_INTEL_SSE_OPT=0", png_set_jump },
    });
    const config_header = b.addConfigHeader(.{
        .style = .{ .cmake = b.path("build/_deps/libpng-src/scripts/pnglibconf.h.prebuilt") },
        .include_path = "pnglibconf.h",
    }, .{});
    png_lib.addConfigHeader(config_header);
    var full_file_path = config_header.getOutput();
    full_file_path.generated.up = 1;
    for (deps) |dep_lib| {
        linkLibrary(png_lib, dep_lib);
    }
    return Library.init(b, png_lib, &.{ include_path, full_file_path }, deps);
}

fn createLibraryZlib(b: *std.Build, options: GraphicsPlaygroundOptions) !Library {
    const zlib_lib = b.addLibrary(.{
        .name = "zlib",
        .root_module = b.createModule(.{
            .target = options.target,
            .optimize = options.optimize,
            .link_libc = true,
        }),
    });
    const include_path = b.path("build/_deps/zlib-src");
    zlib_lib.addIncludePath(include_path);
    zlib_lib.addCSourceFiles(.{ .files = &.{
        "build/_deps/zlib-src/adler32.c",
        "build/_deps/zlib-src/compress.c",
        "build/_deps/zlib-src/crc32.c",
        "build/_deps/zlib-src/deflate.c",
        "build/_deps/zlib-src/gzclose.c",
        "build/_deps/zlib-src/gzlib.c",
        "build/_deps/zlib-src/gzread.c",
        "build/_deps/zlib-src/gzwrite.c",
        "build/_deps/zlib-src/inflate.c",
        "build/_deps/zlib-src/infback.c",
        "build/_deps/zlib-src/inftrees.c",
        "build/_deps/zlib-src/inffast.c",
        "build/_deps/zlib-src/trees.c",
        "build/_deps/zlib-src/uncompr.c",
        "build/_deps/zlib-src/zutil.c",
    }, .flags = &.{"-std=gnu90"} });
    const config_header = b.addConfigHeader(.{
        .style = .{ .cmake = b.path("build/_deps/zlib-src/zconf.h.cmakein") },
        .include_path = "zconf.h",
    }, .{});
    zlib_lib.addConfigHeader(config_header);
    var full_file_path = config_header.getOutput();
    full_file_path.generated.up = 1;
    return Library.init(b, zlib_lib, &.{ include_path, full_file_path }, &.{});
}

fn createLibraryImGUI(b: *std.Build, options: GraphicsPlaygroundOptions, deps: []const *const Library) !Library {
    const imgui_lib = b.addLibrary(.{
        .name = "imgui",
        .root_module = b.createModule(.{
            .target = options.target,
            .optimize = options.optimize,
            .link_libc = true,
        }),
    });
    const include_path = b.path("build/_deps/imgui-src");
    imgui_lib.addIncludePath(include_path);
    imgui_lib.addCSourceFiles(.{
        .files = &.{
            "build/_deps/imgui-src/imgui.cpp",
            "build/_deps/imgui-src/imgui_draw.cpp",
            "build/_deps/imgui-src/imgui_widgets.cpp",
            "build/_deps/imgui-src/imgui_tables.cpp",
            "build/_deps/imgui-src/backends/imgui_impl_opengl3.cpp",
            "build/_deps/imgui-src/backends/imgui_impl_glfw.cpp",
        },
        .flags = &.{"-D IMGUI_IMPL_OPENGL_LOADER_GLAD"},
    });
    for (deps) |dep_lib| {
        linkLibrary(imgui_lib, dep_lib);
    }
    return Library.init(b, imgui_lib, &.{include_path}, deps);
}

fn createLibraryOpenGL(b: *std.Build, options: GraphicsPlaygroundOptions, deps: []const *const Library) !Library {
    const lib = b.addLibrary(.{
        .name = "opengl",
        .root_module = b.createModule(.{
            .target = options.target,
            .optimize = options.optimize,
            .link_libcpp = true,
        }),
    });
    const include_path = b.path("src/libs/opengl");
    lib.addIncludePath(include_path);
    const sources = try findSources(b, "src/libs/opengl", &.{});
    lib.addCSourceFiles(.{ .files = sources, .flags = options.cppFlags() });
    for (deps) |dep_lib| {
        linkLibrary(lib, dep_lib);
    }
    return Library.init(b, lib, &.{include_path}, deps);
}

fn createLibraryCore(b: *std.Build, options: GraphicsPlaygroundOptions, deps: []const *const Library) !Library {
    const core_lib = b.addLibrary(.{
        .name = "core",
        .root_module = b.createModule(.{
            .target = options.target,
            .optimize = options.optimize,
            .link_libcpp = true,
        }),
    });
    const include_path = b.path("src/core");
    core_lib.addIncludePath(include_path);
    const sources = try findSources(b, "src/core", &.{});
    core_lib.addCSourceFiles(.{ .files = sources, .flags = options.cppFlags() });
    for (deps) |dep_lib| {
        linkLibrary(core_lib, dep_lib);
    }
    return Library.init(b, core_lib, &.{include_path}, deps);
}

fn createLibraryFastNoise(b: *std.Build, options: GraphicsPlaygroundOptions, deps: []const *const Library) !Library {
    const fast_noise_lib = b.addLibrary(.{
        .name = "fast_noise",
        .root_module = b.createModule(.{
            .target = options.target,
            .optimize = options.optimize,
            .link_libcpp = true,
        }),
    });
    fast_noise_lib.addIncludePath(b.path("src/libs/fast_noise"));
    fast_noise_lib.addIncludePath(b.path("build/_deps/fastnoise-src/Cpp/"));
    const sources = try findSources(b, "src/libs/fast_noise", &.{});
    fast_noise_lib.addCSourceFiles(.{ .files = sources, .flags = options.cppFlags() });
    for (deps) |dep_lib| {
        linkLibrary(fast_noise_lib, dep_lib);
    }
    return Library.init(b, fast_noise_lib, &.{
        b.path("src/libs/fast_noise"),
        b.path("build/_deps/fastnoise-src/Cpp/"),
    }, deps);
}

fn createLibraryMarchingCubes(b: *std.Build, options: GraphicsPlaygroundOptions, deps: []const *const Library) !Library {
    const marching_cubes_lib = b.addLibrary(.{
        .name = "marching_cubes_lib",
        .root_module = b.createModule(.{
            .target = options.target,
            .optimize = options.optimize,
            .link_libcpp = true,
        }),
    });
    const include_path = b.path("src/libs/marching_cubes");
    marching_cubes_lib.addIncludePath(include_path);
    const sources = try findSources(b, "src/libs/marching_cubes", &.{});
    marching_cubes_lib.addCSourceFiles(.{ .files = sources, .flags = options.cppFlags() });
    for (deps) |dep_lib| {
        linkLibrary(marching_cubes_lib, dep_lib);
    }
    return Library.init(b, marching_cubes_lib, &.{include_path}, deps);
}

fn createLibraryImage(b: *std.Build, options: GraphicsPlaygroundOptions, deps: []const *const Library) !Library {
    const image_lib = b.addLibrary(.{
        .name = "image",
        .root_module = b.createModule(.{
            .target = options.target,
            .optimize = options.optimize,
        }),
    });
    const include_path = b.path("src/libs/image");
    image_lib.addIncludePath(include_path);
    image_lib.addCSourceFile(.{ .file = b.path("src/libs/image/ImageOps.cpp"), .flags = options.cppFlags() });
    image_lib.linkLibCpp();
    for (deps) |dep_lib| {
        linkLibrary(image_lib, dep_lib);
    }
    return Library.init(b, image_lib, &.{include_path}, deps);
}

fn createLibraryModel(b: *std.Build, options: GraphicsPlaygroundOptions, deps: []const *const Library) !Library {
    const model_lib = b.addLibrary(.{
        .name = "model",
        .root_module = b.createModule(.{
            .target = options.target,
            .optimize = options.optimize,
            .link_libcpp = true,
        }),
    });
    const include_path = b.path("src/libs/model");
    model_lib.addIncludePath(include_path);
    model_lib.addCSourceFile(.{
        .file = b.path("src/libs/model/Model.cpp"),
        .flags = options.cppFlags(),
    });
    for (deps) |dep_lib| {
        linkLibrary(model_lib, dep_lib);
    }
    return Library.init(b, model_lib, &.{include_path}, deps);
}

fn createLibraryZip(b: *std.Build, options: GraphicsPlaygroundOptions, deps: []const *const Library) !Library {
    const zip_lib = b.addLibrary(.{
        .name = "zip",
        .root_module = b.createModule(.{
            .target = options.target,
            .optimize = options.optimize,
            .link_libcpp = true,
        }),
    });
    const include_path = b.path("src/libs/zip");
    zip_lib.addIncludePath(include_path);
    zip_lib.addCSourceFiles(.{ .files = &.{
        "src/libs/zip/zip.cpp",
    }, .flags = options.cppFlags() });
    for (deps) |dep_lib| {
        linkLibrary(zip_lib, dep_lib);
    }
    return Library.init(b, zip_lib, &.{include_path}, deps);
}

const Scene = struct {
    name: []const u8,
    sources: []const []const u8,
    deps: []const []const u8 = &.{},
    preferred_optimize_mode: ?std.builtin.OptimizeMode = null,
};

fn createScenes(b: *std.Build, options: GraphicsPlaygroundOptions, default_libs: []const *const Library, libraries: std.StringHashMap(Library)) !void {
    const scenes = [_]Scene{
        .{ .name = "a_star", .sources = &.{ "AStar.cpp", "AStarSolver.cpp" } },
        .{ .name = "ambient_occlusion", .sources = &.{"AmbientOcclusion.cpp"} },
        // .{ .name = "audio_vis", .sources = &.{ "AudioVis.cpp", "WavLoader.cpp" }, .deps = &.{"soundio"} },
        // .{ .name = "bloom_effect", .sources = &.{"BloomEffect.cpp"} },
        .{ .name = "cube", .sources = &.{"Cube.cpp"} },
        .{ .name = "dtm_viewer", .sources = &.{ "DtmViewer.cpp", "DtmDownloader.cpp", "XyzLoader.cpp", "ShpLoader.cpp" }, .deps = &.{ "zlib", "zip", "curl" } },
        // .{ .name = "font_demo", .sources = &.{"FontDemo.cpp"}, .deps = &.{"text"} },
        .{ .name = "fourier_transform", .sources = &.{"FourierTransform.cpp"} },
        .{ .name = "framebuffer_demo", .sources = &.{"FramebufferDemo.cpp"} },
        .{ .name = "game_of_life", .sources = &.{"GameOfLife.cpp"} },
        .{ .name = "gamma_calculation", .sources = &.{"GammaCalculation.cpp"} },
        .{ .name = "graph_vis", .sources = &.{ "GraphVis.cpp", "Graph.cpp" } },
        .{ .name = "graphing_calculator", .sources = &.{"GraphingCalculator.cpp"} },
        .{ .name = "l_systems", .sources = &.{"LSystems.cpp"} },
        .{ .name = "landscape", .sources = &.{ "Landscape.cpp", "Clouds.cpp", "Trees.cpp", "Branch.cpp", "TerrainParams.cpp", "Terrain.cpp" }, .deps = &.{ "image", "model" } },
        .{ .name = "legacy_triangle", .sources = &.{"LegacyTriangle.cpp"} },
        .{ .name = "light_demo", .sources = &.{"LightDemo.cpp"}, .deps = &.{ "image", "model" } },
        .{ .name = "marching_cubes", .sources = &.{"MarchingCubesScene.cpp"}, .deps = &.{ "marching_cubes_lib", "fast_noise" } },
        .{ .name = "meta_balls", .sources = &.{ "MetaBalls.cpp", "MetaBallsScene.cpp" }, .deps = &.{ "marching_cubes_lib", "fast_noise" } },
        // .{ .name = "mine_sweeper", .sources = &.{"MineSweeper.cpp"}, .deps = &.{"text"} },
        .{ .name = "model_loading", .sources = &.{"ModelLoading.cpp"}, .deps = &.{"model"} },
        .{ .name = "normal_mapping", .sources = &.{"NormalMapping.cpp"}, .deps = &.{"model"} },
        .{ .name = "ray_tracing", .sources = &.{ "RayTracing.cpp", "RayTracer.cpp" } },
        .{ .name = "rubiks_cube", .sources = &.{ "AnimationRubiksCube.cpp", "CoreRubiksCube.cpp", "RubiksCubeScene.cpp" } },
        .{ .name = "shadows_2d", .sources = &.{ "RayTracer2D.cpp", "Shadows2D.cpp" } },
        .{ .name = "spot_light", .sources = &.{"SpotLight.cpp"} },
        .{ .name = "terrain_erosion", .sources = &.{ "RainSimulation.cpp", "TerrainErosion.cpp" }, .deps = &.{"fast_noise"}, .preferred_optimize_mode = std.builtin.OptimizeMode.ReleaseFast },
        .{ .name = "tessellation", .sources = &.{"Tessellation.cpp"} },
        .{ .name = "test_scene", .sources = &.{"TestScene.cpp"} },
        .{ .name = "texture_demo", .sources = &.{"TextureDemo.cpp"} },
        .{ .name = "triangle", .sources = &.{"Triangle.cpp"} },
    };
    for (scenes) |scene| {
        std.debug.print("Adding scene: {s}\n", .{scene.name});

        const scene_path = try std.fmt.allocPrint(b.allocator, "src/scenes/{s}", .{scene.name});
        defer b.allocator.free(scene_path);

        const exe = b.addExecutable(.{
            .name = scene.name,
            .root_module = b.createModule(.{
                .target = options.target,
                .optimize = scene.preferred_optimize_mode orelse options.optimize,
            }),
        });

        exe.addIncludePath(b.path(scene_path));
        var sources = std.ArrayList([]const u8).empty;
        defer sources.deinit(b.allocator);
        for (scene.sources) |source_file_path| {
            try sources.append(b.allocator,b.pathJoin(&.{ "src", "scenes", scene.name, source_file_path }));
        }
        exe.addCSourceFiles(.{ .files = sources.items, .flags = options.cppFlags() });

        const shader_sources = try compileShaders(b, scene.name, scene_path);
        defer shader_sources.deinit();
        exe.addCSourceFiles(.{ .files = shader_sources.items, .flags = options.cppFlags() });

        for (default_libs) |lib| {
            linkLibrary(exe, lib);
        }

        for (scene.deps) |dep_name| {
            const dep = libraries.get(dep_name) orelse {
                std.log.err("failed to find library '{s}'", .{dep_name});
                return error.LibraryNotFound;
            };
            linkLibrary(exe, &dep);
        }

        const run_cmd = b.addRunArtifact(exe);
        run_cmd.step.dependOn(b.getInstallStep());

        const run_step = b.step(
            try std.fmt.allocPrint(b.allocator, "run-{s}", .{scene.name}),
            try std.fmt.allocPrint(b.allocator, "Run the {s} scene", .{scene.name}),
        );
        run_step.dependOn(&run_cmd.step);

        b.installArtifact(exe);
    }
}

fn findSources(b: *std.Build, directory: []const u8, ignore_list: []const []const u8) ![][]const u8 {
    var sources = std.ArrayList([]const u8).empty;

    var dir = try std.fs.cwd().openDir(directory, .{ .iterate = true });

    var walker = try dir.walk(b.allocator);
    defer walker.deinit();

    const allowed_exts = [_][]const u8{ ".c", ".cc", ".cpp" };

    while (try walker.next()) |entry| {
        if (entry.kind != std.fs.File.Kind.file) {
            continue;
        }

        const ext = std.fs.path.extension(entry.basename);
        const include_file = for (allowed_exts) |e| {
            if (std.mem.eql(u8, ext, e))
                break true;
        } else false;

        if (!include_file) {
            continue;
        }

        const should_be_ignored =
            for (ignore_list) |ignore_item| {
                if (std.mem.eql(u8, entry.path, ignore_item)) {
                    break true;
                }
            } else false;
        if (should_be_ignored) {
            continue;
        }

        try sources.append(b.allocator, b.pathJoin(&.{ directory, entry.path }));
    }

    return sources.items;
}
