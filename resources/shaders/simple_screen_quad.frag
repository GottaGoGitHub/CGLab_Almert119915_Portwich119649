#version 150

in vec2 pass_TexCoord;

out vec4 out_Color;

uniform sampler2D screenTexture;

uniform vec2 textureSize;

uniform bool horizontalMirroring;
uniform bool verticalMirroring;
uniform bool greyscale;
uniform bool blur;

vec2 tex_coords = pass_TexCoord;

void main() {

    if (horizontalMirroring) {
        tex_coords.y = 1.0-tex_coords.y;
    }

    if (verticalMirroring) {
        tex_coords.x = 1.0-tex_coords.x;
    }

    if (blur) {

        vec2 pixelSize = vec2(1.0, 1.0)/textureSize;

        //neighbours
        vec2 neighbours[9] = vec2[](
        vec2(-pixelSize.x, pixelSize.y), vec2(0.0f, pixelSize.y), vec2(pixelSize.x, pixelSize.y),
        vec2(-pixelSize.x, 0.0f), vec2(0.0f, 0.0f), vec2(pixelSize.x, 0.0f),
        vec2(-pixelSize.x, -pixelSize.y), vec2(0.0f, -pixelSize.y), vec2(pixelSize.x, -pixelSize.y)
        );

        //weighting
        float gaussian_kernel[9] = float[](
        1.0/16, 1.0/8, 1.0/16,
        1.0/8, 1.0/4, 1.0/8,
        1.0/16, 1.0/8, 1.0/16
        );

        //weighted sum of the colors of the neighbouring pixels
        vec3 color = vec3(0.0);
        for (int i = 0; i<9; i++) {
            color += vec3(texture(screenTexture, tex_coords.xy + neighbours[i]))*gaussian_kernel[i];
        }
        out_Color = vec4(color, 1.0);
    }
    else
    {
        out_Color = texture(screenTexture, tex_coords);
    }

    if (greyscale) {
        float luminance = (0.2126*out_Color.r+0.7152*out_Color.g+0.0722*out_Color.b);
        out_Color = vec4(luminance, luminance, luminance, 1.0);
    }
}