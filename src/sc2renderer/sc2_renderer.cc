#include "sc2renderer/sc2_renderer.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <string>

#include <sc2api/sc2_api.h>

#include <stdio.h>

#include <map>
#include <iostream>

namespace {

    const char *const texture_forlder = "textures/";

    const char *const texture_extention = ".png";


    std::map<std::string, SDL_Texture *> textures;

    SDL_Window *window_;
    SDL_Renderer *renderer_;

    SDL_Rect CreateRect(int x, int y, int w, int h) {
        SDL_Rect r;
        r.x = x;
        r.y = y;
        r.w = w;
        r.h = h;
        return r;
    }
}

namespace sc2 {

    namespace renderer {

        void Initialize(const char *title, int x, int y, int w, int h, unsigned int flags) {
            int init_result = SDL_Init(SDL_INIT_VIDEO);
            assert(!init_result);

            window_ = SDL_CreateWindow(title, x, y, w, h, flags == 0 ? SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE : flags);
            assert(window_);

            renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED);
            assert(renderer_);

            // Clear window to black.
            SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 255);
            SDL_RenderClear(renderer_);
        }

        void Shutdown() {
            SDL_DestroyRenderer(renderer_);
            SDL_DestroyWindow(window_);
            SDL_Quit();
        }

        void Matrix1BPP(const char *bytes, int w_mat, int h_mat, int off_x, int off_y, int px_w, int px_h) {
            assert(renderer_);
            assert(window_);

            SDL_Rect rect = CreateRect(0, 0, px_w, px_h);
            for (size_t y = 0; y < h_mat; ++y) {
                for (size_t x = 0; x < w_mat; ++x) {
                    rect.x = off_x + (int(x) * rect.w);
                    rect.y = off_y + (int(y) * rect.h);

                    size_t index = x + y * w_mat;
                    unsigned char mask = 1 << (7 - (index % 8));
                    unsigned char data = bytes[index / 8];
                    bool value = (data & mask) != 0;

                    if (value)
                        SDL_SetRenderDrawColor(renderer_, 255, 255, 255, 255);
                    else
                        SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 255);

                    SDL_RenderFillRect(renderer_, &rect);
                }
            }
        }

        void Matrix8BPPHeightMap(const char *bytes, int w_mat, int h_mat, int off_x, int off_y, int px_w, int px_h) {
            assert(renderer_);
            assert(window_);

            SDL_Rect rect = CreateRect(0, 0, px_w, px_h);
            for (size_t y = 0; y < h_mat; ++y) {
                for (size_t x = 0; x < w_mat; ++x) {
                    rect.x = off_x + (int(x) * rect.w);
                    rect.y = off_y + (int(y) * rect.h);

                    // Renders the height map in grayscale [0-255]
                    size_t index = x + y * w_mat;
                    SDL_SetRenderDrawColor(renderer_, bytes[index], bytes[index], bytes[index], 255);
                    SDL_RenderFillRect(renderer_, &rect);
                }
            }
        }

        void Matrix8BPPPlayers(const char *bytes, int w_mat, int h_mat, int off_x, int off_y, int px_w, int px_h) {
            assert(renderer_);
            assert(window_);

            SDL_Rect rect = CreateRect(0, 0, px_w, px_h);
            for (size_t y = 0; y < h_mat; ++y) {
                for (size_t x = 0; x < w_mat; ++x) {
                    rect.x = off_x + (int(x) * rect.w);
                    rect.y = off_y + (int(y) * rect.h);

                    size_t index = x + y * w_mat;
                    switch (bytes[index]) {
                        case 0:
                            SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 255);
                            break;
                        case 1:
                            // Self.
                            SDL_SetRenderDrawColor(renderer_, 0, 255, 0, 255);
                            break;
                        case 2:
                            // Enemy.
                            SDL_SetRenderDrawColor(renderer_, 255, 0, 0, 255);
                            break;
                        case 3:
                            // Neutral.
                            SDL_SetRenderDrawColor(renderer_, 0, 0, 255, 255);
                            break;
                        case 4:
                            SDL_SetRenderDrawColor(renderer_, 255, 255, 0, 255);
                            break;
                        case 5:
                            SDL_SetRenderDrawColor(renderer_, 0, 255, 255, 255);
                            break;
                        default:
                            SDL_SetRenderDrawColor(renderer_, 255, 255, 255, 255);
                    }
                    SDL_RenderFillRect(renderer_, &rect);
                }
            }
        }


        /**
         * To avoid loading the texture from disk each time, we load them into a cache.
         * If it is not availiable in the cache, we load the image from disk, otherwise we return it.
         * All units' image must follow a pattern to be loaded dynamically using the unit name in sc2_typesenums.h
         * Pattern is RACE_UNIT[_STATE]
         * i.e:
         * TERRAN_SCV
         * There is a pack of png images
         * https://www.reddit.com/r/starcraft/comments/1axm20/hires_starcraft_2_units_structures_pngs_with/
         */
        SDL_Texture *LoadUnitTexture(const sc2::Unit *unit) {

            SDL_Texture *unit_texture;
            SDL_Surface *unit_image;

            std::string unit_name = sc2::UnitTypeToName(unit->unit_type);

            unit_texture = textures[unit_name];

            if (unit_texture != nullptr) {
                return unit_texture;
            }

            std::string unit_image_path = texture_forlder + unit_name + texture_extention;
            unit_image = IMG_Load(unit_image_path.c_str());
            unit_texture = SDL_CreateTextureFromSurface(renderer_, unit_image);
            SDL_FreeSurface(unit_image);

            textures[unit_name] = unit_texture;

            return unit_texture;
        }


        /**
         * Render all the units in the observation thingy to the screem
         */
        void RenderUnits(const sc2::ObservationInterface *obs) {
            assert(renderer_);
            assert(window_);

            // TODO Manage alliances and colors
            // TODO Manage the facing by rendering different portion of the texture depending on the unit->facing value
            // DONE Use a cache of texture to avoid reloading them from the disk at each call
            // TODO Make the image path configurable so we can add subdirectories to have customisable textures
            // TODO Manage state of units (i.e carrying minerals or vespen) by adding _STATE to the unit name and texture.png
            // TODO Manage rescaling of higher res textures to the size of units (using the radius as basis and the FeatureLayerSettings)
            // TODO Manage camera pos
            // TODO Display HP / shield / energy above the units

            for (const auto &unit : obs->GetUnits()) {
                obs->GetCameraPos();

                if (unit->alliance == sc2::Unit::Self) {
                    SDL_Texture *unit_texture = LoadUnitTexture(unit);

                    if(unit_texture == nullptr) {
                        std::cout<< "No texture found for "<< sc2::UnitTypeToName(unit->unit_type) <<std::endl;
                        break;
                    }

                    Uint32 texture_format = 0;
                    int texture_access = 0;
                    int texture_w = 0;
                    int texture_h = 0;
                    SDL_QueryTexture(unit_texture, &texture_format, &texture_access, &texture_w, &texture_h);

                    // facing is the orientation -> select the part of the texture that matches the case.
                    // unit->facing;

                    /*
                     * Just using a single texture for now. needs to be implemented to have moving attacking mining animations.
                     * Most likely to have a two dimensional array of texture in a .png.
                     * Each sub texture should have the same size so we can easly cut it into subtextures
                     * On x axis we have different orientation
                     * On y we have different movement for each orientation and render a bit of each?
                     * And different .png files for each state, attacks and such?
                     */

                    // TODO use FeatureLayerSettings to get the basis sizes
                    //float sub_texture_index = sub_texture_size * (unit->facing * 8 % (3.14*2) );
                    //float sub_texture_size = texture_w / 8;
                    float sub_texture_index = 0;
                    int on_screen_w = 100 * unit->radius;
                    int on_screen_h = texture_h * on_screen_w / texture_w;

                    // rescale the source texture for rendering
                    SDL_Rect source_texture = {sub_texture_index, 0, texture_w, texture_h};
                    SDL_Rect on_screen_texture = {unit->pos.x, unit->pos.y, on_screen_w, on_screen_h};

                    SDL_RenderCopy(renderer_, unit_texture, &source_texture, &on_screen_texture);


                } else if (unit->alliance == sc2::Unit::Enemy) {
                    // render in different color?
                }

                }

        }

        /**
         * We would need to make and HUD to display info about the game (minerals, gaz, pop, actions on current selected unit)
         */
        void RenderHUD(const sc2::ObservationInterface *obs) {

            assert(renderer_);
            assert(window_);


/*
        std::cout << "\nPlayerID:" << client.Observation()->GetPlayerID();
        std::cout << "\nMinerals:" << client.Observation()->GetMinerals();
*/


        }

        SDL_Event Render() {
            assert(renderer_);
            assert(window_);

            SDL_RenderPresent(renderer_);
            SDL_RenderClear(renderer_);

            SDL_Event e;
            while (SDL_PollEvent(&e)) {


                if(e.type == SDL_QUIT) {
                    Shutdown();
                } else {
                    return e;
                }

            }

        }

    }

}