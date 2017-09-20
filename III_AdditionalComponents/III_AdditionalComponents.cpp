#include <plugin_III.h>
#include "game_III\CClumpModelInfo.h"
#include "game_III\common.h"
#include "game_III\CWeather.h"
#include "extensions\KeyCheck.h"
#include "game_III\CMessages.h"

#include "game_III\CFont.h"

using namespace plugin;

class AdditionalComponents {
public:
    enum eWiperState { ONE_STATE_LEFT, ONE_STATE_RIGHT, TWO_STATE_LEFT, TWO_STATE_RIGHT };
    
    static eWiperState m_currentWiperOneState, m_currentWiperTwoState;
    static float wiperOneAngle, wiperTwoLAngle, wiperTwoRAngle;
    static CWeather *wather;

    class VehicleComponents { // �����, ������� ������������ ���� ������ (����� �������, ��� ��� ������ "�������������" � ��������� ����������)
    public:
        RwFrame *m_pBootSliding, *m_pSteerWheel, *m_pBootMirage, *m_pWiperOneR, *m_pWiperOneL, *m_pWiperOneM;
        RwFrame *m_pWiperTwoR, *m_pWiperTwoL, *m_pWiperOneTwoR, *m_pWiperOneTwoL, *m_pBrushOneR, *m_pBrushOneL;
        RwFrame *m_pDumper;
        bool wiperState;
        float dumperAngle;

        VehicleComponents(CVehicle *) { // ����������� ����� ������ ����� ������ ��� ������ ������������ ���������� (CVehicle::CVehicle)
            m_pBootSliding = m_pBootMirage = m_pSteerWheel = m_pWiperOneR = m_pWiperOneL = m_pWiperOneM = nullptr;  // ������������� ��� ��������� � 0
            m_pWiperTwoR = m_pWiperTwoL = m_pWiperOneTwoR = m_pWiperOneTwoL = m_pBrushOneR = m_pBrushOneL = m_pDumper = nullptr;
            wiperState = false;
            dumperAngle = 0.0f;

        }
    };

    static void MatrixSetRotateXOnly(RwFrame *component, float angle) {
        CMatrix matrix(&component->modelling, false);
        matrix.SetRotateXOnly(angle);
        matrix.UpdateRW();
    }

    static void MatrixSetRotateYOnly(RwFrame *component, float angle) {
        CMatrix matrix(&component->modelling, false);
        matrix.SetRotateYOnly(angle);
        matrix.UpdateRW();
    }

    static void WiperWorks(CVehicle *vehicle, float angle) {
        MatrixSetRotateYOnly(vehComps.Get(vehicle).m_pWiperOneR, wiperOneAngle);
        MatrixSetRotateYOnly(vehComps.Get(vehicle).m_pWiperOneL, wiperOneAngle);
        if (vehComps.Get(vehicle).m_pWiperOneM)
            MatrixSetRotateYOnly(vehComps.Get(vehicle).m_pWiperOneM, wiperOneAngle);
        if (vehComps.Get(vehicle).m_pWiperOneTwoR)
            MatrixSetRotateYOnly(vehComps.Get(vehicle).m_pWiperOneTwoR, wiperOneAngle);
        if (vehComps.Get(vehicle).m_pWiperOneTwoL)
            MatrixSetRotateYOnly(vehComps.Get(vehicle).m_pWiperOneTwoL, wiperOneAngle);
        if (vehComps.Get(vehicle).m_pBrushOneR)
            MatrixSetRotateYOnly(vehComps.Get(vehicle).m_pBrushOneR, -wiperOneAngle);
        if (vehComps.Get(vehicle).m_pBrushOneL)
            MatrixSetRotateYOnly(vehComps.Get(vehicle).m_pBrushOneL, -wiperOneAngle);
    }

    static VehicleExtendedData<VehicleComponents> vehComps; // ������� ��������� ������ ����������. vehComps - ��� ���������� ����� ������� �� ����� ���������� � ����� ������, ��������� ����� Get(CVehicle *���������) 

    AdditionalComponents() {

        Events::vehicleSetModelEvent += [](CVehicle *vehicle, int modelIndex) { // ��������� ���� �������, ����� ���� ������������� ������ ����������
            if (vehicle->m_pRwClump) { // ������� ���������� � �������� � ���������� �� � ��� �����
                vehComps.Get(vehicle).m_pBootSliding  = CClumpModelInfo::GetFrameFromName(vehicle->m_pRwClump, "boot_sliding");
                vehComps.Get(vehicle).m_pBootMirage   = CClumpModelInfo::GetFrameFromName(vehicle->m_pRwClump, "boot_mirage");
                vehComps.Get(vehicle).m_pSteerWheel   = CClumpModelInfo::GetFrameFromName(vehicle->m_pRwClump, "steerwheel"); 
                vehComps.Get(vehicle).m_pWiperOneR    = CClumpModelInfo::GetFrameFromName(vehicle->m_pRwClump, "wiper_or");
                vehComps.Get(vehicle).m_pWiperOneL    = CClumpModelInfo::GetFrameFromName(vehicle->m_pRwClump, "wiper_ol");
                vehComps.Get(vehicle).m_pWiperOneM    = CClumpModelInfo::GetFrameFromName(vehicle->m_pRwClump, "wiper_om");
                vehComps.Get(vehicle).m_pWiperTwoR    = CClumpModelInfo::GetFrameFromName(vehicle->m_pRwClump, "wiper_tr");
                vehComps.Get(vehicle).m_pWiperTwoL    = CClumpModelInfo::GetFrameFromName(vehicle->m_pRwClump, "wiper_tl");
                vehComps.Get(vehicle).m_pWiperOneTwoR = CClumpModelInfo::GetFrameFromName(vehicle->m_pRwClump, "wiper_otr");
                vehComps.Get(vehicle).m_pWiperOneTwoL = CClumpModelInfo::GetFrameFromName(vehicle->m_pRwClump, "wiper_otl");
                vehComps.Get(vehicle).m_pBrushOneR    = CClumpModelInfo::GetFrameFromName(vehicle->m_pRwClump, "brush_or");
                vehComps.Get(vehicle).m_pBrushOneL    = CClumpModelInfo::GetFrameFromName(vehicle->m_pRwClump, "brush_ol");
                vehComps.Get(vehicle).m_pDumper       = CClumpModelInfo::GetFrameFromName(vehicle->m_pRwClump, "dumper");

            }
            else {
                vehComps.Get(vehicle).m_pBootSliding = vehComps.Get(vehicle).m_pSteerWheel = vehComps.Get(vehicle).m_pWiperOneR = vehComps.Get(vehicle).m_pWiperOneL = nullptr;
                vehComps.Get(vehicle).m_pBootMirage = vehComps.Get(vehicle).m_pWiperOneM = vehComps.Get(vehicle).m_pWiperTwoR = vehComps.Get(vehicle).m_pWiperTwoL = nullptr;
                vehComps.Get(vehicle).m_pWiperOneTwoR = vehComps.Get(vehicle).m_pWiperOneTwoL = vehComps.Get(vehicle).m_pBrushOneR = vehComps.Get(vehicle).m_pBrushOneL = nullptr;
                vehComps.Get(vehicle).m_pDumper = nullptr;

            }
        };

        Events::gameProcessEvent += [] {
            for (int i = 0; i < CPools::ms_pVehiclePool->m_nSize; i++) {
                CVehicle *vehicle = CPools::ms_pVehiclePool->GetAt(i);
                if (vehicle && vehicle->GetIsOnScreen() && vehicle->m_fHealth > 0.0f && vehicle->m_nVehicleClass == VEHICLE_AUTOMOBILE) {
                    CAutomobile *automobile = reinterpret_cast<CAutomobile *>(vehicle);
                    // trunk shutters
                    if (vehicle->m_pDriver && vehComps.Get(vehicle).m_pBootSliding) {
                        if (automobile->m_aCarNodes[CAR_BOOT] && automobile->m_aCarNodes[CAR_WING_LR]) {
                            if (automobile->m_carDamage.GetPanelStatus(WING_REAR_LEFT) == 0) {
                                automobile->m_carDamage.SetDoorStatus(BOOT, 0);
                                vehComps.Get(vehicle).m_pBootSliding->modelling.at.z = 1.0f + automobile->m_aDoors[1].m_fAngle;
                            }
                            else 
                                automobile->m_carDamage.SetDoorStatus(BOOT, 3);
                        }
                    }
                    // trunk with wipers
                    if (vehComps.Get(vehicle).m_pBootMirage && automobile->m_aCarNodes[CAR_BOOT]) {
                        if ((automobile->m_aDoors[1].m_fAngle < 0.0f) && (vehicle->m_nVehicleFlags & 0x20)) 
                            MatrixSetRotateXOnly(automobile->m_aCarNodes[CAR_BOOT], automobile->m_aDoors[1].m_fAngle);
                        MatrixSetRotateXOnly(vehComps.Get(vehicle).m_pBootMirage, automobile->m_aDoors[1].m_fAngle);
                        RwV3d scaleBootMirage;
                        if (automobile->m_carDamage.GetDoorStatus(BOOT) == 3) 
                            scaleBootMirage = { 0.0f, 0.0f, 0.0f };
                        else 
                            scaleBootMirage = { 1.0f, 1.0f, 1.0f };
                        RwFrameScale(vehComps.Get(vehicle).m_pBootMirage, &scaleBootMirage, rwCOMBINEPRECONCAT);
                    }
                    // steering wheel
                    if (vehicle->m_pDriver && vehComps.Get(vehicle).m_pSteerWheel) {
                        MatrixSetRotateYOnly(vehComps.Get(vehicle).m_pSteerWheel, (vehicle->m_fSteerAngle * (-7.0f)));
                    }
                    // wipers type one
                    if (vehComps.Get(vehicle).m_pWiperOneR && vehComps.Get(vehicle).m_pWiperOneL) {
                        if (vehicle->m_pDriver && wather->NewWeatherType == 2) 
                            vehComps.Get(vehicle).wiperState = true;
                        if (vehComps.Get(vehicle).wiperState == true) {
                            if (m_currentWiperOneState == ONE_STATE_LEFT) {
                                wiperOneAngle -= 0.05f;
                                if (wiperOneAngle > -1.45f) 
                                    WiperWorks(vehicle, wiperOneAngle);
                                else {
                                    wiperOneAngle = -1.4f;
                                    m_currentWiperOneState = ONE_STATE_RIGHT;
                                }
                            }
                            else {
                                wiperOneAngle += 0.05f;
                                if (wiperOneAngle < 0.05f) 
                                    WiperWorks(vehicle, wiperOneAngle);
                                else {
                                    wiperOneAngle = 0.0f;
                                    if (!vehicle->m_pDriver)
                                        vehComps.Get(vehicle).wiperState = true;
                                    else
                                        vehComps.Get(vehicle).wiperState = false;
                                    m_currentWiperOneState = ONE_STATE_LEFT;
                                }
                            }
                        }
                    }
                    // wipers type two
                    if (vehComps.Get(vehicle).m_pWiperTwoR && vehComps.Get(vehicle).m_pWiperTwoL) {
                        if (vehicle->m_pDriver && wather->NewWeatherType == 2)
                            vehComps.Get(vehicle).wiperState = true;
                        if (vehComps.Get(vehicle).wiperState == true) {
                            if (m_currentWiperTwoState == TWO_STATE_LEFT) {
                                wiperTwoLAngle -= 0.05f;
                                if (wiperTwoLAngle > -1.15f)
                                    MatrixSetRotateYOnly(vehComps.Get(vehicle).m_pWiperTwoL, wiperTwoLAngle);
                                if (wiperTwoLAngle < -0.25f) {
                                    wiperTwoRAngle -= 0.05f;
                                    if (wiperTwoRAngle > -1.15f) 
                                        MatrixSetRotateYOnly(vehComps.Get(vehicle).m_pWiperTwoR, wiperTwoRAngle);
                                    else {
                                        wiperTwoLAngle = wiperTwoRAngle = -1.1f;
                                        m_currentWiperTwoState = TWO_STATE_RIGHT;
                                    }
                                }
                            }
                            else {
                                wiperTwoRAngle += 0.05f;
                                if (wiperTwoRAngle < 0.05f)
                                    MatrixSetRotateYOnly(vehComps.Get(vehicle).m_pWiperTwoR, wiperTwoRAngle);
                                if (wiperTwoRAngle > -0.85f) {
                                    wiperTwoLAngle += 0.05f;
                                    if (wiperTwoLAngle < 0.05f)
                                        MatrixSetRotateYOnly(vehComps.Get(vehicle).m_pWiperTwoL, wiperTwoLAngle);
                                    else {
                                        wiperTwoLAngle = wiperTwoRAngle = 0.0f;
                                        if (!vehicle->m_pDriver)
                                            vehComps.Get(vehicle).wiperState = true;
                                        else
                                            vehComps.Get(vehicle).wiperState = false;
                                        m_currentWiperTwoState = TWO_STATE_LEFT;
                                    }
                                }
                            }
                        }
                    }
                    //

                }
            }
        };

        Events::drawingEvent += [] {
            CVehicle *playerVehicle = FindPlayerVehicle();
            if (playerVehicle && playerVehicle->m_nVehicleClass == VEHICLE_AUTOMOBILE) {
                CAutomobile *playerAutomobile = reinterpret_cast<CAutomobile *>(playerVehicle);
                // dumper
                if (vehComps.Get(playerVehicle).m_pDumper) {
                    if (KeyPressed(0x6E) && KeyPressed(0x68)) {
                        if (vehComps.Get(playerVehicle).dumperAngle < 0.7f) {
                            vehComps.Get(playerVehicle).dumperAngle += 0.01f;
                            MatrixSetRotateXOnly(vehComps.Get(playerVehicle).m_pDumper, vehComps.Get(playerVehicle).dumperAngle);
                        }
                        else 
                            CMessages::AddMessageJumpQ(L"full open", 1000, 0);
                    }
                    if (KeyPressed(0x6E) && KeyPressed(0x62)) {
                        if (vehComps.Get(playerVehicle).dumperAngle > 0.0f) {
                            vehComps.Get(playerVehicle).dumperAngle -= 0.01f;
                            MatrixSetRotateXOnly(vehComps.Get(playerVehicle).m_pDumper, vehComps.Get(playerVehicle).dumperAngle);
                        }
                        else
                            CMessages::AddMessageJumpQ(L"full closed", 1000, 0);
                    }
                }
                // 
                
                
                //KeyCheck::Update();
                //if (KeyCheck::CheckWithDelay(0xBE, 500)) {
                //    if (automobile->m_aDoors[1].IsClosed()) {
                //        //vehicle->m_nVehicleFlags &= 0xEF;
                //        //RwV3d axis = { 1.0f, 0.0f, 0.0f };
                //        //RwFrameRotate(automobile->m_aCarNodes[CAR_BOOT], &axis, 1.0f, rwCOMBINEPRECONCAT);
                //        //RwMatrixRotate(&automobile->m_aCarNodes[CAR_BOOT]->modelling, &axis, 1.0f, rwCOMBINEPRECONCAT);
                //        CMessages::AddMessageJumpQ(L"IsClosed", 2000, 0);
                //    }
                //    else {
                //        //vehicle->m_nVehicleFlags |= 0x10;
                //        //RwV3d axis = { 1.0f, 0.0f, 0.0f };
                //        //RwFrameRotate(automobile->m_aCarNodes[CAR_BOOT], &axis, 0.0f, rwCOMBINEPRECONCAT);
                //        //RwMatrixRotate(&automobile->m_aCarNodes[CAR_BOOT]->modelling, &axis, 0.0f, rwCOMBINEPRECONCAT);
                //        CMessages::AddMessageJumpQ(L"Open", 2000, 0);
                //    }
                //}

                //CPed *player = FindPlayerPed();
                //if (player) {
                // ����� ������
                CFont::SetScale(0.5f, 1.0f);
                CFont::SetColor(CRGBA(255, 255, 255, 255));
                CFont::SetJustifyOn();
                CFont::SetFontStyle(0);
                CFont::SetPropOn();
                CFont::SetWrapx(600.0f);
                wchar_t text[32];
                //swprintf(text, L"steer angle %.2f", playerVehicle->m_fSteerAngle);
                int currentWather = wather->NewWeatherType;
                switch (currentWather) {
                case 0:
                    swprintf(text, L"weather %hs", "SUNNY");
                    break;
                case 1:
                    swprintf(text, L"weather %hs", "CLOUDY");
                    break;
                case 2:
                    swprintf(text, L"weather %hs", "RAINY");
                    break;
                case 3:
                    swprintf(text, L"weather %hs", "FOGGY");
                    break;
                }
                //swprintf(text, L"weather %d", wather->NewWeatherType);
                CFont::PrintString(10.0f, 10.0f, text);
                //swprintf(text, L"dvig %d", playerVehicle->m_nVehicleFlags);
                //CFont::PrintString(10.0f, 30.0f, text);
                //swprintf(text, L"OpenRatio %1f", playerAutomobile->m_aDoors[1].GetAngleOpenRatio());
                //CFont::PrintString(10.0f, 50.0f, text);
                //swprintf(text, L"Angle %.2f", playerAutomobile->m_aDoors[1].m_fAngle);
                //CFont::PrintString(10.0f, 80.0f, text);
                //swprintf(text, L"PrevAngle %.2f", playerAutomobile->m_aDoors[1].m_fPrevAngle);
                //CFont::PrintString(10.0f, 110.0f, text);
            }
        };

    }
} AdditionalComponents;

VehicleExtendedData<AdditionalComponents::VehicleComponents> AdditionalComponents::vehComps;
AdditionalComponents::eWiperState AdditionalComponents::m_currentWiperOneState = ONE_STATE_LEFT;
AdditionalComponents::eWiperState AdditionalComponents::m_currentWiperTwoState = TWO_STATE_LEFT;
float AdditionalComponents::wiperOneAngle = 0.0f;
float AdditionalComponents::wiperTwoLAngle = 0.0f;
float AdditionalComponents::wiperTwoRAngle = 0.0f;
