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
    include_path: std.Build.LazyPath,
};

pub fn build(b: *std.Build) !void {
    const options = GraphicsPlaygroundOptions{
        .target = b.standardTargetOptions(.{}),
        .optimize = b.standardOptimizeOption(.{}),
        .enable_asan = b.option(bool, "enable-asan", "Enable address sanitizer") orelse false,
        .warnings_as_errors = b.option(bool, "warnings-as-errors", "Treat warnings as errors") orelse false,
    };

    const glfw = b.dependency("glfw", .{
        .target = options.target,
        .optimize = options.optimize,
    });

    const glad_lib = try createLibraryGlad(b, options);
    const glfw_lib = Library{ .lib = glfw.artifact("glfw"), .include_path = glfw.path("") };
    const imgui_lib = try createLibraryImGUI(b, options, &glad_lib, &glfw_lib);
    const opengl_lib = try createLibraryOpenGL(b, options, &glad_lib);
    const core_lib = try createLibraryCore(b, options, &glad_lib, &opengl_lib, &glfw_lib, &imgui_lib);

    try createScenes(b, options, &core_lib, &.{ &glad_lib, &opengl_lib, &imgui_lib });
}

fn compileShader(b: *std.Build, scene_name: []const u8, shader_path: []const u8, output_dir: []const u8) ![]const u8 {
    std.debug.print("Compiling shader: {s} -> {s}\n", .{ shader_path, output_dir });

    const shader_file = try fs.cwd().openFile(shader_path, .{});
    defer shader_file.close();

    var buf_reader = std.io.bufferedReader(shader_file.reader());
    var in_stream = buf_reader.reader();

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
    std.debug.print("Compiling shaders in: {s}\n", .{scene_path});

    var dir = try std.fs.cwd().openDir(scene_path, .{ .iterate = true });
    defer dir.close();

    var walker = try dir.walk(b.allocator);
    defer walker.deinit();

    var shader_paths = std.ArrayList([]const u8).init(b.allocator);
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
        try shader_paths.append(source_file_path);
    }

    return shader_paths;
}

fn linkLibrary(step: *std.Build.Step.Compile, lib: *const Library) void {
    step.linkLibrary(lib.lib);
    step.addIncludePath(lib.include_path);
}

fn createLibraryGlad(b: *std.Build, options: GraphicsPlaygroundOptions) !Library {
    const glad_lib = b.addStaticLibrary(.{
        .name = "glad",
        .target = options.target,
        .optimize = options.optimize,
    });
    const include_path = b.path("src/libs/glad/include");
    glad_lib.addIncludePath(include_path);
    const sources = try findSources(b, "src/libs/glad/src", &.{});
    glad_lib.addCSourceFiles(.{ .files = sources });
    glad_lib.linkLibC();
    return .{ .lib = glad_lib, .include_path = include_path };
}

fn createLibraryImGUI(b: *std.Build, options: GraphicsPlaygroundOptions, glad_lib: *const Library, glfw_lib: *const Library) !Library {
    const imgui_lib = b.addStaticLibrary(.{
        .name = "glfw",
        .target = options.target,
        .optimize = options.optimize,
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
    linkLibrary(imgui_lib, glad_lib);
    linkLibrary(imgui_lib, glfw_lib);
    imgui_lib.linkLibC();
    return .{ .lib = imgui_lib, .include_path = include_path };
}

fn createLibraryOpenGL(b: *std.Build, options: GraphicsPlaygroundOptions, glad_lib: *const Library) !Library {
    const lib = b.addStaticLibrary(.{
        .name = "opengl",
        .target = options.target,
        .optimize = options.optimize,
    });
    const include_path = b.path("src/libs/opengl");
    lib.addIncludePath(include_path);
    const sources = try findSources(b, "src/libs/opengl", &.{});
    lib.addCSourceFiles(.{ .files = sources, .flags = options.cppFlags() });
    linkLibrary(lib, glad_lib);
    lib.linkLibCpp();
    return .{ .lib = lib, .include_path = include_path };
}

fn createLibraryCore(b: *std.Build, options: GraphicsPlaygroundOptions, glad_lib: *const Library, opengl_lib: *const Library, glfw_lib: *const Library, imgui_lib: *const Library) !Library {
    const core_lib = b.addStaticLibrary(.{
        .name = "core",
        .target = options.target,
        .optimize = options.optimize,
    });
    const include_path = b.path("src/core");
    core_lib.addIncludePath(include_path);
    const sources = try findSources(b, "src/core", &.{});
    core_lib.addCSourceFiles(.{ .files = sources, .flags = options.cppFlags() });
    core_lib.linkLibCpp();
    linkLibrary(core_lib, glad_lib);
    linkLibrary(core_lib, opengl_lib);
    linkLibrary(core_lib, glfw_lib);
    linkLibrary(core_lib, imgui_lib);
    return .{ .lib = core_lib, .include_path = include_path };
}

fn createScenes(b: *std.Build, options: GraphicsPlaygroundOptions, core_lib: *const Library, libs: []const *const Library) !void {
    const scenes_directory = "src/scenes";
    var dir = try std.fs.cwd().openDir(scenes_directory, .{ .iterate = true });

    var walker = try dir.walk(b.allocator);
    defer walker.deinit();

    var scenes = std.ArrayList([]const u8).init(b.allocator);
    defer scenes.deinit();
    while (try walker.next()) |entry| {
        if (entry.kind != std.fs.File.Kind.directory) {
            continue;
        }

        std.debug.print("Looking at directory: {s}\n", .{entry.path});

        // include top-level directories
        if (std.mem.indexOfScalar(u8, entry.path, std.fs.path.sep)) |_| {
            continue;
        }

        try scenes.append(try b.allocator.dupe(u8, entry.path));
    }

    // Create an executable for each scene
    for (scenes.items) |scene_name| {
        std.debug.print("Adding scene: {s}\n", .{scene_name});
        if (!std.mem.eql(u8, scene_name, "triangle") and !std.mem.eql(u8, scene_name, "legacy_triangle")) {
            continue;
        }

        const scene_path = try std.fmt.allocPrint(b.allocator, "src/scenes/{s}", .{scene_name});
        defer b.allocator.free(scene_path);

        const exe = b.addExecutable(.{
            .name = scene_name,
            .target = options.target,
            .optimize = options.optimize,
        });

        exe.addIncludePath(b.path(scene_path));
        const sources = try findSources(b, scene_path, &.{});
        exe.addCSourceFiles(.{ .files = sources, .flags = options.cppFlags() });

        const source_file_paths = try compileShaders(b, scene_name, scene_path);
        defer source_file_paths.deinit();
        exe.addCSourceFiles(.{ .files = source_file_paths.items, .flags = options.cppFlags() });

        linkLibrary(exe, core_lib);
        for (libs) |lib| {
            linkLibrary(exe, lib);
        }

        const run_cmd = b.addRunArtifact(exe);
        run_cmd.step.dependOn(b.getInstallStep());

        const run_step = b.step(
            try std.fmt.allocPrint(b.allocator, "run-{s}", .{scene_name}),
            try std.fmt.allocPrint(b.allocator, "Run the {s} scene", .{scene_name}),
        );
        run_step.dependOn(&run_cmd.step);

        b.installArtifact(exe);
    }
}

fn findSources(b: *std.Build, directory: []const u8, ignore_list: []const []const u8) ![][]const u8 {
    var sources = std.ArrayList([]const u8).init(b.allocator);

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

        try sources.append(b.pathJoin(&.{ directory, entry.path }));
    }

    return sources.items;
}
