//
// Created by per on 11/3/17.
//

#include <sc2utils/sc2_manage_process.h>
#include <sc2api/sc2_api.h>
#include <sc2renderer/sc2_renderer.h>
#include <iostream>
#include <SDL2/SDL.h>


const float kCameraWidth = 24.0f;
const int kFeatureLayerSize = 80;
const int kPixelDrawSize = 5;
const int kDrawSize = kFeatureLayerSize * kPixelDrawSize;

void DrawFeatureLayer1BPP(const SC2APIProtocol::ImageData &image_data, int off_x, int off_y) {
    assert(image_data.bits_per_pixel() == 1);
    int width = image_data.size().x();
    int height = image_data.size().y();
    sc2::renderer::Matrix1BPP(image_data.data().c_str(), width, height, off_x, off_y, kPixelDrawSize, kPixelDrawSize);
}

void DrawFeatureLayerUnits8BPP(const SC2APIProtocol::ImageData &image_data, int off_x, int off_y) {
    assert(image_data.bits_per_pixel() == 8);
    int width = image_data.size().x();
    int height = image_data.size().y();
    sc2::renderer::Matrix8BPPPlayers(image_data.data().c_str(), width, height, off_x, off_y, kPixelDrawSize,
                                     kPixelDrawSize);
}

void DrawFeatureLayerHeightMap8BPP(const SC2APIProtocol::ImageData &image_data, int off_x, int off_y) {
    assert(image_data.bits_per_pixel() == 8);
    int width = image_data.size().x();
    int height = image_data.size().y();
    sc2::renderer::Matrix8BPPHeightMap(image_data.data().c_str(), width, height, off_x, off_y, kPixelDrawSize,
                                       kPixelDrawSize);
}

class HumanController : public sc2::Agent {
    sc2::Point2D* camera_position;

public:
    void ManageInput(SDL_Event event) {
        const sc2::ObservationInterface *obs = Observation();

        const Uint8 *keys = SDL_GetKeyboardState(NULL);

        float delta_x = 0;
        float delta_y = 0;
        if (keys[SDL_SCANCODE_LEFT]) {
            delta_x -= 0.1;
        } else if (keys[SDL_SCANCODE_RIGHT]) {
            delta_x += 0.1;
        } else if (keys[SDL_SCANCODE_UP]) {
            delta_y -= 0.1;
        } else if (keys[SDL_SCANCODE_DOWN]) {
            delta_y += 0.1;
        }

        if(delta_x != 0 || delta_y != 0) {

            camera_position->x = camera_position->x+delta_x;
            camera_position->y = camera_position->y+delta_y;

        }

        switch (event.type) {
            case SDL_MOUSEBUTTONDOWN :

                int x, y;
                SDL_GetMouseState(&x, &y);
                // TODO rescale correctly the position of the mouse according to the draw size of the window
                sc2::Point2D target(x / kPixelDrawSize, y / kPixelDrawSize);

                for (const auto &unit : obs->GetUnits()) {
                    if (unit->alliance == sc2::Unit::Self) {
                        Actions()->UnitCommand(unit, sc2::ABILITY_ID::MOVE, target);
                    }

                }

                break;

            default:
                break;
        }

    }

    virtual

    void OnGameStart() final {
        Debug()->DebugTextOut("Human");
        Debug()->SendDebug();
        sc2::renderer::Initialize("StarCraft II", 50, 50, 2 * kDrawSize, 2 * kDrawSize);
    }

    virtual void OnStep() final {
        const SC2APIProtocol::Observation *observation = Observation()->GetRawObservation();

        const SC2APIProtocol::FeatureLayersMinimap &mi = observation->feature_layer_data().minimap_renders();
        DrawFeatureLayerHeightMap8BPP(mi.height_map(), 0, kDrawSize);
        DrawFeatureLayer1BPP(mi.camera(), kDrawSize, kDrawSize);

        Control()->GetObservation();

        const sc2::ObservationInterface *obs = Observation();

        // TODO draw the floor of the map in the back ground.
        sc2::renderer::RenderBackGround(camera_position);

        // TODO see things to be done in the function
        sc2::renderer::RenderUnits(obs, camera_position);
        // TODO implement the HUD
        sc2::renderer::RenderHUD(obs);


        SDL_Event event = sc2::renderer::Render();

        ManageInput(event);
    }

    virtual void OnGameEnd() final {
        sc2::renderer::Shutdown();
    }

};


// TODO Customisation of settings
int main(int argc, char *argv[]) {
    sc2::Coordinator coordinator;
    if (!coordinator.LoadSettings(argc, argv)) {
        return 1;
    }

    sc2::FeatureLayerSettings settings(kCameraWidth, kFeatureLayerSize, kFeatureLayerSize, kFeatureLayerSize,
                                       kFeatureLayerSize);
    coordinator.SetFeatureLayers(settings);

    // Add the custom bot, it will control the players.
    HumanController controller;

    coordinator.SetParticipants({
                                        CreateParticipant(sc2::Race::Terran, &controller),
                                        CreateComputer(sc2::Race::Zerg)
                                });

    // Start the game.
    coordinator.LaunchStarcraft();

    coordinator.StartGame(sc2::kMapBelShirVestigeLE);

    while (coordinator.Update()) {

        if (sc2::PollKeyPress()) {
            break;
        }
    }

    return 0;
}