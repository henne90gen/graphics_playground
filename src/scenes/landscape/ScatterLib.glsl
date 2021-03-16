const float PI = 3.14159265358979323846;
#if 0
const vec3 _bR = vec3(1.95e-4, 1.1e-3, 2.94e-3);
const float _bM = 4e-5;
const float _g = 0.93F;
#elif 1
const vec3 _bR = vec3(5.5e-6, 13.0e-6, 22.4e-6);
const float _bM = 2.1e-5;
const float _g = 0.75f;
#else
const vec3 _bR = vec3(5.8e-6, 13.5e-6, 33.1e-6);
const float _bM = 210.0e-5;
const float _g = 0.76F;
#endif

// http://developer.amd.com/wordpress/media/2012/10/GDC_02_HoffmanPreetham.pdf
// https://www.scratchapixel.com/lessons/procedural-generation-virtual-worlds/simulating-sky/simulating-colors-of-the-sky
vec4 calcScattering(vec3 cameraPosition, vec3 modelPosition, vec3 lightDirection, vec3 lightColor, float lightPower, vec3 atmosphere,
vec4 color) {
    vec3 bR = _bR * atmosphere.x;
    float bM = _bM * atmosphere.y;
    float g = _g * atmosphere.z;

    vec3 modelToCamera = cameraPosition - modelPosition;
    float s = length(modelToCamera);
    vec3 extinction = exp(-(bR + bM) * s);

    float cosTheta = dot(modelToCamera, lightDirection) / (length(modelToCamera) * length(lightDirection));
    float cR = 3 / (16*PI);
    float cM = 1 / (4*PI);
    vec3 bRTheta = cR * bR * (1 + cosTheta*cosTheta);
    float oneMinusGSq = (1-g) * (1-g);
    float bMTheta = cM * bM * (oneMinusGSq / pow(1 + g*g - 2*g*cosTheta, 3/2));
    vec3 t1 = (bRTheta + bMTheta) / (bR + bM);
    vec3 t2 = 1 - extinction;
    vec3 inScatter = t1 * lightColor * lightPower * t2;

    return color * vec4(extinction, 1.0F) + vec4(inScatter, 1.0F);
}

vec3 rotate_vector(vec4 quat, vec3 vec) {
    return vec + 2.0 * cross(cross(vec, quat.xyz) + quat.w * vec, quat.xyz);
}

// https://www.shadertoy.com/view/wlBXWK
vec3 calculate_scattering(
vec3 start, // the start of the ray (the camera position)
vec3 dir, // the direction of the ray (the camera vector)
float max_dist, // the maximum distance the ray can travel (because something is in the way, like an object)

vec4 cameraOrientation, // HACK we are using the camera orientation to position the planet correctly, since the given start position and direction are in camera space

vec3 scene_color, // the color of the scene
vec3 light_dir, // the direction of the light
vec3 light_intensity// how bright the light is, affects the brightness of the atmosphere
) {
    float planet_radius = 371e3;// the radius of the planet (used to be 6371e3)
    float atmo_radius = 471e3;// the radius of the atmosphere (used to be 6471e3)
    vec3 planet_position = vec3(0.0, -(planet_radius+10), 0.0);// the position of the planet

    vec3 beta_ray = vec3(5.5e-6, 13.0e-6, 22.4e-6);// the amount rayleigh scattering scatters the colors (for earth: causes the blue atmosphere)
    vec3 beta_mie = vec3(21e-6);// the amount mie scattering scatters colors
    vec3 beta_absorption = vec3(2.04e-5, 4.97e-5, 1.95e-6);// how much air is absorbed
    vec3 beta_ambient = vec3(0.0);// the amount of scattering that always occurs, can help make the back side of the atmosphere a bit brighter
    float g = 0.7;// the direction mie scatters the light in (like a cone). closer to -1 means more towards a single direction

    float height_ray = 8e3;// how high do you have to go before there is no rayleigh scattering?
    float height_mie = 1.2e3;// the same, but for mie
    float height_absorption = 30e3;// the height at which the most absorption happens
    float absorption_falloff = 3e3;// how fast the absorption falls off from the absorption height

    int steps_primary = 64;// the amount of steps along the 'primary' ray, more looks better but slower
    int steps_light = 4;// the amount of steps along the light ray, more looks better but slower


    planet_position = rotate_vector(cameraOrientation, planet_position);

    // add an offset to the camera position, so that the atmosphere is in the correct position
    start -= planet_position;
    // calculate the start and end position of the ray, as a distance along the ray
    // we do this with a ray sphere intersect
    float a = dot(dir, dir);
    float b = 2.0 * dot(dir, start);
    float c = dot(start, start) - (atmo_radius * atmo_radius);
    float d = (b * b) - 4.0 * a * c;

    // stop early if there is no intersect
    if (d < 0.0) return scene_color;

    // calculate the ray length
    vec2 ray_length = vec2(
    max((-b - sqrt(d)) / (2.0 * a), 0.0),
    min((-b + sqrt(d)) / (2.0 * a), max_dist)
    );

    // if the ray did not hit the atmosphere, return a black color
    if (ray_length.x > ray_length.y) return scene_color;
    // prevent the mie glow from appearing if there's an object in front of the camera
    bool allow_mie = max_dist > ray_length.y;
    // make sure the ray is no longer than allowed
    ray_length.y = min(ray_length.y, max_dist);
    ray_length.x = max(ray_length.x, 0.0);
    // get the step size of the ray
    float step_size_primary = (ray_length.y - ray_length.x) / float(steps_primary);

    // next, set how far we are along the ray, so we can calculate the position of the sample
    // if the camera is outside the atmosphere, the ray should start at the edge of the atmosphere
    // if it's inside, it should start at the position of the camera
    // the min statement makes sure of that
    float ray_pos_i = ray_length.x;

    // these are the values we use to gather all the scattered light
    vec3 total_ray = vec3(0.0);// for rayleigh
    vec3 total_mie = vec3(0.0);// for mie

    // initialize the optical depth. This is used to calculate how much air was in the ray
    vec3 opt_i = vec3(0.0);

    // also init the scale height, avoids some vec2's later on
    vec2 scale_height = vec2(height_ray, height_mie);

    // Calculate the Rayleigh and Mie phases.
    // This is the color that will be scattered for this ray
    // mu, mumu and gg are used quite a lot in the calculation, so to speed it up, precalculate them
    float mu = dot(dir, light_dir);
    float mumu = mu * mu;
    float gg = g * g;
    float phase_ray = 3.0 / (50.2654824574 /* (16 * pi) */) * (1.0 + mumu);
    float phase_mie = allow_mie ? 3.0 / (25.1327412287 /* (8 * pi) */) * ((1.0 - gg) * (mumu + 1.0)) / (pow(1.0 + gg - 2.0 * mu * g, 1.5) * (2.0 + gg)) : 0.0;

    // now we need to sample the 'primary' ray. this ray gathers the light that gets scattered onto it
    for (int i = 0; i < steps_primary; ++i) {

        // calculate where we are along this ray
        vec3 pos_i = start + dir * (ray_pos_i + step_size_primary * 0.5);

        // and how high we are above the surface
        float height_i = length(pos_i) - planet_radius;

        // now calculate the density of the particles (both for rayleigh and mie)
        vec3 density = vec3(exp(-height_i / scale_height), 0.0);

        // and the absorption density. this is for ozone, which scales together with the rayleigh,
        // but absorbs the most at a specific height, so use the sech function for a nice curve falloff for this height
        // clamp it to avoid it going out of bounds. This prevents weird black spheres on the night side
        density.z = clamp((1.0 / cosh((height_absorption - height_i) / absorption_falloff)) * density.x, 0.0, 1.0);
        density *= step_size_primary;

        // Add these densities to the optical depth, so that we know how many particles are on this ray.
        opt_i += density;

        // Calculate the step size of the light ray.
        // again with a ray sphere intersect
        // a, b, c and d are already defined
        a = dot(light_dir, light_dir);
        b = 2.0 * dot(light_dir, pos_i);
        c = dot(pos_i, pos_i) - (atmo_radius * atmo_radius);
        d = (b * b) - 4.0 * a * c;

        // no early stopping, this one should always be inside the atmosphere
        // calculate the ray length
        float step_size_light = (-b + sqrt(d)) / (2.0 * a * float(steps_light));

        // and the position along this ray
        // this time we are sure the ray is in the atmosphere, so set it to 0
        float ray_pos_l = 0.0;

        // and the optical depth of this ray
        vec3 opt_l = vec3(0.0);

        // now sample the light ray
        // this is similar to what we did before
        for (int l = 0; l < steps_light; ++l) {

            // calculate where we are along this ray
            vec3 pos_l = pos_i + light_dir * (ray_pos_l + step_size_light * 0.5);

            // the heigth of the position
            float height_l = length(pos_l) - planet_radius;

            // calculate the particle density, and add it
            vec3 density_l = vec3(exp(-height_l / scale_height), 0.0);
            density_l.z = clamp((1.0 / cosh((height_absorption - height_l) / absorption_falloff)) * density_l.x, 0.0, 1.0);
            opt_l += density_l * step_size_light;

            // and increment where we are along the light ray.
            ray_pos_l += step_size_light;

        }

        // Now we need to calculate the attenuation
        // this is essentially how much light reaches the current sample point due to scattering
        vec3 attn = exp(-(beta_mie * (opt_i.y + opt_l.y) + beta_ray * (opt_i.x + opt_l.x) + beta_absorption * (opt_i.z + opt_l.z)));

        // accumulate the scattered light (how much will be scattered towards the camera)
        total_ray += density.x * attn;
        total_mie += density.y * attn;

        // and increment the position on this ray
        ray_pos_i += step_size_primary;

    }

    // calculate how much light can pass through the atmosphere
    vec3 opacity = exp(-(beta_mie * opt_i.y + beta_ray * opt_i.x + beta_absorption * opt_i.z));

    // calculate and return the final color
    return (
    phase_ray * beta_ray * total_ray// rayleigh color
    + phase_mie * beta_mie * total_mie// mie
    + opt_i.x * beta_ambient// and ambient
    ) * light_intensity + scene_color * opacity;// now make sure the background is rendered correctly
}


