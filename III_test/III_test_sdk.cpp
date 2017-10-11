#include "plugin_III.h"
#include "game_III\common.h"
#include "extensions\KeyCheck.h"
#include "game_III\CMessages.h"
#include "game_III\CFont.h"

//#include "game_III\CModelInfo.h"

using namespace plugin;

class MyPlugin {
public:
    MyPlugin() {
        //CModelInfo::AddVehicleModel(4000);
        // 0x546B99;
        
        Events::drawingEvent += [] {
            //if (*(bool *)0x8F5AE9) {
            //    // ����� ������
            //    CFont::SetScale(0.5f, 1.0f);
            //    CFont::SetColor(CRGBA(255, 255, 255, 255));
            //    CFont::SetJustifyOn();
            //    CFont::SetFontStyle(0);
            //    CFont::SetPropOn();
            //    CFont::SetWrapx(600.0f);
            //    wchar_t text[32];
            //    swprintf(text, L"test %hs", "777");
            //    CFont::PrintString(100.0f, 100.0f, text);
            //}

            CVehicle *playerVehicle = FindPlayerVehicle();
            if (playerVehicle && playerVehicle->m_nVehicleClass == VEHICLE_AUTOMOBILE) {
                CAutomobile *playerAutomobile = reinterpret_cast<CAutomobile *>(playerVehicle);

               /* for (int i = 0; i < CPools::ms_pVehiclePool->m_nSize; i++) {
                    CVehicle *vehicle = CPools::ms_pVehiclePool->GetAt(i);
                    if (vehicle && !vehicle->m_pDriver && vehicle->m_fHealth > 0.1f && vehicle->m_nVehicleClass == VEHICLE_AUTOMOBILE) {
                        if ((playerVehicle->m_matrix.pos.x - vehicle->m_matrix.pos.x) < 6.0f) {
                            float posX = playerVehicle->m_matrix.pos.x;
                            vehicle->m_matrix.pos.x = posX - 3.0f;
                            vehicle->m_matrix.pos.y = playerVehicle->m_matrix.pos.y - 3.0f;
                            vehicle->m_matrix.pos.z = playerVehicle->m_matrix.pos.z;
                            vehicle->m_matrix.at.z = playerVehicle->m_matrix.at.z;
                        }
                    }
                }*/
                    
                //CPed *player = FindPlayerPed();
                //if (player) 
                // ����� ������
                CFont::SetScale(0.5f, 1.0f);
                CFont::SetColor(CRGBA(255, 255, 255, 255));
                CFont::SetJustifyOn();
                CFont::SetFontStyle(0);
                CFont::SetPropOn();
                CFont::SetWrapx(600.0f);
                wchar_t text[32];
                /*int currentWather = wather->NewWeatherType;
                switch (currentWather) {
                case 0: swprintf(text, L"weather %hs", "SUNNY");  break;
                case 1: swprintf(text, L"weather %hs", "CLOUDY"); break;
                case 2: swprintf(text, L"weather %hs", "RAINY");  break;
                case 3: swprintf(text, L"weather %hs", "FOGGY");  break;
                }*/
                //CFont::PrintString(10.0f, 10.0f, text);
                
                
                swprintf(text, L"m_fAngle 2 %.2f", playerAutomobile->m_aDoors[2].m_fAngle);
                CFont::PrintString(10.0f, 30.0f, text);
                swprintf(text, L"m_fAngle 3 %.2f", playerAutomobile->m_aDoors[3].m_fAngle);
                CFont::PrintString(10.0f, 50.0f, text);
                //swprintf(text, L"steer angle %.2f", playerVehicle->m_fSteerAngle);
                /*swprintf(text, L"dumperAngle %.2f", vehComps.Get(playerVehicle).dumperAngle);
                CFont::PrintString(10.0f, 50.0f, text);
                swprintf(text, L"manholePos %.2f", vehComps.Get(playerVehicle).manholePos);
                CFont::PrintString(10.0f, 80.0f, text);*/
                //KeyCheck::Update();
                //if (KeyCheck::CheckWithDelay(0x6E, 1000))
                //    //playerVehicle->BurstTyre(0);
                //    playerVehicle->m_matrix.SetRotateZOnly(1.57f);

                /*swprintf(text, L"WheelRot %d", playerAutomobile->m_nTireFriction[0]);
                CFont::PrintString(10.0f, 110.0f, text);
                swprintf(text, L"WheelRot %d", playerAutomobile->m_nTireFriction[1]);
                CFont::PrintString(10.0f, 140.0f, text);
                swprintf(text, L"WheelRot %d", playerAutomobile->m_nTireFriction[2]);
                CFont::PrintString(10.0f, 170.0f, text);
                swprintf(text, L"WheelRot %d", playerAutomobile->m_nTireFriction[3]);
                CFont::PrintString(10.0f, 200.0f, text);*/

                /*swprintf(text, L"LightStatus %d", playerAutomobile->m_carDamage.GetLightStatus(LIGHT_FRONT_LEFT));
                CFont::PrintString(10.0f, 140.0f, text);
                swprintf(text, L"LightStatus %d", playerAutomobile->m_carDamage.GetLightStatus(LIGHT_FRONT_RIGHT));
                CFont::PrintString(10.0f, 170.0f, text);
                swprintf(text, L"LightStatus %d", playerAutomobile->m_carDamage.GetLightStatus(LIGHT_REAR_RIGHT));
                CFont::PrintString(10.0f, 200.0f, text);
                swprintf(text, L"LightStatus %d", playerAutomobile->m_carDamage.GetLightStatus(LIGHT_REAR_LEFT));
                CFont::PrintString(10.0f, 230.0f, text);*/
            }
        };

    }
} myPlugin;

