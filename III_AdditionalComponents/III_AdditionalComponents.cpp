#include <plugin_III.h>
#include "game_III\CClumpModelInfo.h"
#include "game_III\common.h"
#include "game_III\CWeather.h"
#include "extensions\KeyCheck.h"
#include "game_III\CMessages.h"

#include <vector>
#include <string>
#include <sstream>
#include <fstream>

using namespace plugin;
using namespace std;

class AdditionalComponents {
public:
    struct Zr350Info {
        unsigned int baseModelId;
        float valueMaxLightsAngle;
    };

    static vector<Zr350Info>& GetZr350Infos() {
        static vector<Zr350Info> zr350Infos;
        return zr350Infos;
    }

    static vector<unsigned int>& GetKeys() {
        static vector<unsigned int> key;
        return key;
    }

    static void ReadSettingsFile() {
        ifstream stream("AdditionalComponents.dat");
        for (string line; getline(stream, line); ) {
            if (line[0] != ';' && line[0] != '#') {
                if (!line.compare("zr350")) {
                    while (getline(stream, line) && line.compare("end")) {
                        if (line[0] != ';' && line[0] != '#') {
                            Zr350Info entry;
                            if (sscanf(line.c_str(), "%d, %f", &entry.baseModelId, &entry.valueMaxLightsAngle) == 2)
                                GetZr350Infos().push_back(entry);
                        }
                    }
                }
                else if (!line.compare("keys")) {
                    while (getline(stream, line) && line.compare("end")) {
                        if (line[0] != ';' && line[0] != '#') {
                            stringstream ss(line);
                            int i;
                            while (ss >> i) {
                                GetKeys().push_back(i);
                                if (ss.peek() == ',')
                                    ss.ignore();
                            }
                        }
                    }
                }
            }
        }
    }

    static Zr350Info *GetZr350InfoForModel(unsigned int BaseModelId) {
        for (unsigned int i = 0; i < GetZr350Infos().size(); i++) {
            if (GetZr350Infos()[i].baseModelId == BaseModelId)
                return &GetZr350Infos()[i];
        }
        return nullptr;
    }
    
    enum eWiperState { ONE_STATE_LEFT, ONE_STATE_RIGHT, TWO_STATE_LEFT, TWO_STATE_RIGHT };
    enum eOpenManholeState { STATE_OPENING, STATE_CLOSING, STATE_WAITING };

    static eOpenManholeState m_currentManholeState;
    static eWiperState m_currentWiperOneState, m_currentWiperTwoState;
    static float wiperOneAngle, wiperTwoLAngle, wiperTwoRAngle, cementAngle, manholeAngle;
    static CWeather *wather;

    class VehicleComponents { 
    public:
        RwFrame *m_pBootSliding, *m_pSteerWheel, *m_pBootMirage, *m_pWiperOneR, *m_pWiperOneL, *m_pWiperOneM, 
            *m_pWiperTwoR, *m_pWiperTwoL, *m_pWiperOneTwoR, *m_pWiperOneTwoL, *m_pBrushOneR, *m_pBrushOneL, 
            *m_pDumper, *m_pCement, *m_pManhole_af, *m_pManhole_ab, *m_pManhole_bf, *m_pManhole_bb, *m_pManhole_cf, 
            *m_pManhole_cb, *m_pManhole_s, *m_pLightsUp;
        bool wiperState, cementState, manholeState, lightsUpDown;
        float dumperAngle, manholePos, currentLightsAngle, maxLightsAngle;

        VehicleComponents(CVehicle *) { 
            m_pBootSliding = m_pBootMirage = m_pSteerWheel = m_pWiperOneR = m_pWiperOneL = m_pWiperOneM = m_pCement 
                = m_pWiperTwoR = m_pWiperTwoL = m_pWiperOneTwoR = m_pWiperOneTwoL = m_pBrushOneR = m_pBrushOneL 
                = m_pDumper = m_pManhole_af = m_pManhole_ab = m_pManhole_bf = m_pManhole_bb = m_pManhole_cf 
                = m_pManhole_cb = m_pManhole_s = m_pLightsUp = nullptr;
            wiperState = false; cementState = true; manholeState = true; lightsUpDown = false;
            dumperAngle = manholePos = currentLightsAngle = 0.0f;

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

    static void ManholeWorks(CVehicle *vehicle, float angle) {
        if (vehComps.Get(vehicle).m_pManhole_af)
            MatrixSetRotateXOnly(vehComps.Get(vehicle).m_pManhole_af, manholeAngle);
        if (vehComps.Get(vehicle).m_pManhole_ab)
            MatrixSetRotateXOnly(vehComps.Get(vehicle).m_pManhole_ab, -manholeAngle);
        if (vehComps.Get(vehicle).m_pManhole_bf)
            MatrixSetRotateXOnly(vehComps.Get(vehicle).m_pManhole_bf, manholeAngle);
        if (vehComps.Get(vehicle).m_pManhole_bb)
            MatrixSetRotateXOnly(vehComps.Get(vehicle).m_pManhole_bb, -manholeAngle);
        if (vehComps.Get(vehicle).m_pManhole_cf)
            MatrixSetRotateXOnly(vehComps.Get(vehicle).m_pManhole_cf, manholeAngle);
        if (vehComps.Get(vehicle).m_pManhole_cb)
            MatrixSetRotateXOnly(vehComps.Get(vehicle).m_pManhole_cb, -manholeAngle);
    }

    static VehicleExtendedData<VehicleComponents> vehComps; // ������� ��������� ������ ����������. vehComps - ��� ���������� ����� ������� �� ����� ���������� � ����� ������, ��������� ����� Get(CVehicle *���������) 

    AdditionalComponents() {
        ReadSettingsFile();

        Events::vehicleSetModelEvent += [](CVehicle *vehicle, int modelIndex) { 
            if (vehicle) {
                Zr350Info *entryModel = GetZr350InfoForModel(vehicle->m_nModelIndex);
                if (entryModel) {
                    CAutomobile *automobile = reinterpret_cast<CAutomobile *>(vehicle);
                    automobile->m_carDamage.SetLightStatus(LIGHT_FRONT_LEFT, 1);
                    automobile->m_carDamage.SetLightStatus(LIGHT_FRONT_RIGHT, 1);
                    vehComps.Get(vehicle).maxLightsAngle = entryModel->valueMaxLightsAngle;
                }
            }
            
            if (vehicle->m_pRwClump) { 
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
                vehComps.Get(vehicle).m_pCement       = CClumpModelInfo::GetFrameFromName(vehicle->m_pRwClump, "cement");
                vehComps.Get(vehicle).m_pManhole_af   = CClumpModelInfo::GetFrameFromName(vehicle->m_pRwClump, "hatch_af_hi");
                vehComps.Get(vehicle).m_pManhole_ab   = CClumpModelInfo::GetFrameFromName(vehicle->m_pRwClump, "hatch_ab_hi");
                vehComps.Get(vehicle).m_pManhole_bf   = CClumpModelInfo::GetFrameFromName(vehicle->m_pRwClump, "hatch_bf_hi");
                vehComps.Get(vehicle).m_pManhole_bb   = CClumpModelInfo::GetFrameFromName(vehicle->m_pRwClump, "hatch_bb_hi");
                vehComps.Get(vehicle).m_pManhole_cf   = CClumpModelInfo::GetFrameFromName(vehicle->m_pRwClump, "hatch_cf_hi");
                vehComps.Get(vehicle).m_pManhole_cb   = CClumpModelInfo::GetFrameFromName(vehicle->m_pRwClump, "hatch_cb_hi");
                vehComps.Get(vehicle).m_pManhole_s    = CClumpModelInfo::GetFrameFromName(vehicle->m_pRwClump, "hatch_s_hi");
                vehComps.Get(vehicle).m_pLightsUp     = CClumpModelInfo::GetFrameFromName(vehicle->m_pRwClump, "lights_up");

            }
            else {
                vehComps.Get(vehicle).m_pBootSliding = vehComps.Get(vehicle).m_pSteerWheel = vehComps.Get(vehicle).m_pWiperOneR 
                    = vehComps.Get(vehicle).m_pWiperOneL = vehComps.Get(vehicle).m_pBootMirage = vehComps.Get(vehicle).m_pWiperOneM 
                    = vehComps.Get(vehicle).m_pWiperTwoR = vehComps.Get(vehicle).m_pWiperTwoL = vehComps.Get(vehicle).m_pWiperOneTwoR 
                    = vehComps.Get(vehicle).m_pWiperOneTwoL = vehComps.Get(vehicle).m_pBrushOneR = vehComps.Get(vehicle).m_pBrushOneL 
                    = vehComps.Get(vehicle).m_pDumper = vehComps.Get(vehicle).m_pCement = vehComps.Get(vehicle).m_pManhole_af 
                    = vehComps.Get(vehicle).m_pManhole_ab = vehComps.Get(vehicle).m_pManhole_bf = vehComps.Get(vehicle).m_pManhole_bb
                    = vehComps.Get(vehicle).m_pManhole_cf = vehComps.Get(vehicle).m_pManhole_cb = vehComps.Get(vehicle).m_pManhole_s
                    = vehComps.Get(vehicle).m_pLightsUp = nullptr;

            }
        };

        Events::gameProcessEvent += [] {
            for (int i = 0; i < CPools::ms_pVehiclePool->m_nSize; i++) {
                CVehicle *vehicle = CPools::ms_pVehiclePool->GetAt(i);
                if (vehicle &&  vehicle->m_fHealth > 0.1f && vehicle->m_nVehicleClass == VEHICLE_AUTOMOBILE) {
                    CAutomobile *automobile = reinterpret_cast<CAutomobile *>(vehicle);
                    if (vehicle->GetIsOnScreen()) {
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
                                    if (wiperOneAngle > -1.4f)
                                        WiperWorks(vehicle, wiperOneAngle);
                                    else {
                                        wiperOneAngle = -1.4f;
                                        WiperWorks(vehicle, wiperOneAngle);
                                        m_currentWiperOneState = ONE_STATE_RIGHT;
                                    }
                                }
                                else {
                                    wiperOneAngle += 0.05f;
                                    if (wiperOneAngle < 0.0f)
                                        WiperWorks(vehicle, wiperOneAngle);
                                    else {
                                        wiperOneAngle = 0.0f;
                                        WiperWorks(vehicle, wiperOneAngle);
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
                                    if (wiperTwoLAngle > -1.1f)
                                        MatrixSetRotateYOnly(vehComps.Get(vehicle).m_pWiperTwoL, wiperTwoLAngle);
                                    if (wiperTwoLAngle < -0.25f) {
                                        wiperTwoRAngle -= 0.05f;
                                        if (wiperTwoRAngle > -1.1f)
                                            MatrixSetRotateYOnly(vehComps.Get(vehicle).m_pWiperTwoR, wiperTwoRAngle);
                                        else {
                                            wiperTwoLAngle = wiperTwoRAngle = -1.1f;
                                            MatrixSetRotateYOnly(vehComps.Get(vehicle).m_pWiperTwoL, wiperTwoLAngle);
                                            MatrixSetRotateYOnly(vehComps.Get(vehicle).m_pWiperTwoR, wiperTwoRAngle);
                                            m_currentWiperTwoState = TWO_STATE_RIGHT;
                                        }
                                    }
                                }
                                else {
                                    wiperTwoRAngle += 0.05f;
                                    if (wiperTwoRAngle < 0.0f)
                                        MatrixSetRotateYOnly(vehComps.Get(vehicle).m_pWiperTwoR, wiperTwoRAngle);
                                    if (wiperTwoRAngle > -0.85f) {
                                        wiperTwoLAngle += 0.05f;
                                        if (wiperTwoLAngle < 0.0f)
                                            MatrixSetRotateYOnly(vehComps.Get(vehicle).m_pWiperTwoL, wiperTwoLAngle);
                                        else {
                                            wiperTwoLAngle = wiperTwoRAngle = 0.0f;
                                            MatrixSetRotateYOnly(vehComps.Get(vehicle).m_pWiperTwoL, wiperTwoLAngle);
                                            MatrixSetRotateYOnly(vehComps.Get(vehicle).m_pWiperTwoR, wiperTwoRAngle);
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
                        // cement
                        if (vehComps.Get(vehicle).m_pCement && (vehComps.Get(vehicle).cementState == true) && (vehicle->m_nVehicleFlags & 0x10)) {
                            cementAngle += 0.05f;
                            MatrixSetRotateYOnly(vehComps.Get(vehicle).m_pCement, cementAngle);
                        }
                        // manhole
                        if (vehicle->m_pDriver) {
                            if (wather->NewWeatherType == 0 && (vehComps.Get(vehicle).manholeState == true)) {
                                manholeAngle = 0.12f;
                                ManholeWorks(vehicle, manholeAngle);
                                vehComps.Get(vehicle).manholeState = false;
                            }
                            else if ((wather->NewWeatherType == 2 || wather->NewWeatherType == 3) && (vehComps.Get(vehicle).manholeState == false)) {
                                manholeAngle = 0.0f;
                                ManholeWorks(vehicle, manholeAngle);
                                vehComps.Get(vehicle).manholeState = true;
                            }
                        }
                        // manhole sliding
                        if (vehicle->m_pDriver && vehComps.Get(vehicle).m_pManhole_s) {
                            CPed *player = FindPlayerPed();
                            if (player && player != vehicle->m_pDriver) {
                                switch (m_currentManholeState) {
                                case STATE_OPENING:
                                    vehComps.Get(vehicle).manholePos -= 0.01f;
                                    if (vehComps.Get(vehicle).manholePos > -0.5f)
                                        vehComps.Get(vehicle).m_pManhole_s->modelling.pos.y = vehComps.Get(vehicle).manholePos;
                                    else {
                                        vehComps.Get(vehicle).manholePos = -0.5f;
                                        vehComps.Get(vehicle).m_pManhole_s->modelling.pos.y = vehComps.Get(vehicle).manholePos;
                                        m_currentManholeState = STATE_WAITING;
                                    }
                                    break;
                                case STATE_CLOSING:
                                    vehComps.Get(vehicle).manholePos += 0.01f;
                                    if (vehComps.Get(vehicle).manholePos < 0.0f)
                                        vehComps.Get(vehicle).m_pManhole_s->modelling.pos.y = vehComps.Get(vehicle).manholePos;
                                    else {
                                        vehComps.Get(vehicle).manholePos = 0.0f;
                                        vehComps.Get(vehicle).m_pManhole_s->modelling.pos.y = vehComps.Get(vehicle).manholePos;
                                        m_currentManholeState = STATE_WAITING;
                                    }
                                    break;
                                case STATE_WAITING:
                                    if (wather->NewWeatherType == 0 && vehComps.Get(vehicle).manholePos == 0.0f)
                                        m_currentManholeState = STATE_OPENING;
                                    else if ((wather->NewWeatherType == 2 || wather->NewWeatherType == 3) && vehComps.Get(vehicle).manholePos == -0.5f)
                                        m_currentManholeState = STATE_CLOSING;
                                    break;
                                }
                            }
                        } 
                        //

                    } // vehicle->GetIsOnScreen
                    
                    // lights up/down
                    if (vehicle->m_pDriver && vehComps.Get(vehicle).m_pLightsUp) {
                        if (vehicle->m_nVehicleFlags & 0x40 ) {
                            if (vehComps.Get(vehicle).lightsUpDown == false) {
                                vehComps.Get(vehicle).currentLightsAngle += 0.1f;
                                if (vehComps.Get(vehicle).currentLightsAngle < vehComps.Get(vehicle).maxLightsAngle)
                                    MatrixSetRotateXOnly(vehComps.Get(vehicle).m_pLightsUp, vehComps.Get(vehicle).currentLightsAngle);
                                else {
                                    vehComps.Get(vehicle).currentLightsAngle = vehComps.Get(vehicle).maxLightsAngle;
                                    MatrixSetRotateXOnly(vehComps.Get(vehicle).m_pLightsUp, vehComps.Get(vehicle).currentLightsAngle);
                                    automobile->m_carDamage.SetLightStatus(LIGHT_FRONT_LEFT, 0);
                                    automobile->m_carDamage.SetLightStatus(LIGHT_FRONT_RIGHT, 0);
                                    vehComps.Get(vehicle).lightsUpDown = true;
                                }
                            }
                        }
                        else if (vehComps.Get(vehicle).lightsUpDown == true) {
                            automobile->m_carDamage.SetLightStatus(LIGHT_FRONT_LEFT, 1);
                            automobile->m_carDamage.SetLightStatus(LIGHT_FRONT_RIGHT, 1);
                            vehComps.Get(vehicle).currentLightsAngle -= 0.1f;
                            if (vehComps.Get(vehicle).currentLightsAngle > 0.0f)
                                MatrixSetRotateXOnly(vehComps.Get(vehicle).m_pLightsUp, vehComps.Get(vehicle).currentLightsAngle);
                            else {
                                vehComps.Get(vehicle).currentLightsAngle = 0.0f;
                                MatrixSetRotateXOnly(vehComps.Get(vehicle).m_pLightsUp, vehComps.Get(vehicle).currentLightsAngle);
                                vehComps.Get(vehicle).lightsUpDown = false;
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
                    //if (KeyPressed(0x6E) && KeyPressed(0x68)) {
                    if (KeyPressed(GetKeys()[0]) && KeyPressed(GetKeys()[1])) {
                        vehComps.Get(playerVehicle).dumperAngle += 0.01f;
                        if (vehComps.Get(playerVehicle).dumperAngle < 0.7f) 
                            MatrixSetRotateXOnly(vehComps.Get(playerVehicle).m_pDumper, vehComps.Get(playerVehicle).dumperAngle);
                        else {
                            CMessages::AddMessageJumpQ(L"full up", 1000, 0);
                            vehComps.Get(playerVehicle).dumperAngle = 0.7f;
                            MatrixSetRotateXOnly(vehComps.Get(playerVehicle).m_pDumper, vehComps.Get(playerVehicle).dumperAngle);
                        }
                    }
                    //if (KeyPressed(0x6E) && KeyPressed(0x62)) {
                    if (KeyPressed(GetKeys()[0]) && KeyPressed(GetKeys()[2])) {
                        vehComps.Get(playerVehicle).dumperAngle -= 0.01f;
                        if (vehComps.Get(playerVehicle).dumperAngle > 0.0f) 
                            MatrixSetRotateXOnly(vehComps.Get(playerVehicle).m_pDumper, vehComps.Get(playerVehicle).dumperAngle);
                        else {
                            CMessages::AddMessageJumpQ(L"full down", 1000, 0);
                            vehComps.Get(playerVehicle).dumperAngle = 0.0f;
                            MatrixSetRotateXOnly(vehComps.Get(playerVehicle).m_pDumper, vehComps.Get(playerVehicle).dumperAngle);
                        }
                    }
                }
                // cement 
                KeyCheck::Update();
                if (KeyCheck::CheckWithDelay(GetKeys()[3], 1000) && vehComps.Get(playerVehicle).m_pCement) {
                    if (vehComps.Get(playerVehicle).cementState == true)
                        vehComps.Get(playerVehicle).cementState = false;
                    else
                        vehComps.Get(playerVehicle).cementState = true;
                }
                // manhole sliding
                if (vehComps.Get(playerVehicle).m_pManhole_s) {
                    //if (KeyPressed(0x6E) && KeyPressed(0x68)) {
                    if (KeyPressed(GetKeys()[0]) && KeyPressed(GetKeys()[1])) {
                        vehComps.Get(playerVehicle).manholePos -= 0.01f;
                        if (vehComps.Get(playerVehicle).manholePos > -0.5f) 
                            vehComps.Get(playerVehicle).m_pManhole_s->modelling.pos.y = vehComps.Get(playerVehicle).manholePos;
                        else {
                            CMessages::AddMessageJumpQ(L"full open", 1000, 0);
                            vehComps.Get(playerVehicle).manholePos = -0.5f;
                            vehComps.Get(playerVehicle).m_pManhole_s->modelling.pos.y = vehComps.Get(playerVehicle).manholePos;
                        }
                    }
                    //if (KeyPressed(0x6E) && KeyPressed(0x62)) {
                    if (KeyPressed(GetKeys()[0]) && KeyPressed(GetKeys()[2])) {
                        vehComps.Get(playerVehicle).manholePos += 0.01f;
                        if (vehComps.Get(playerVehicle).manholePos < 0.0f) 
                            vehComps.Get(playerVehicle).m_pManhole_s->modelling.pos.y = vehComps.Get(playerVehicle).manholePos;
                        else {
                            CMessages::AddMessageJumpQ(L"full closed", 1000, 0);
                            vehComps.Get(playerVehicle).manholePos = 0.0f;
                            vehComps.Get(playerVehicle).m_pManhole_s->modelling.pos.y = vehComps.Get(playerVehicle).manholePos;
                        }
                    }
                }
                //

            }
        };

    }
} AdditionalComponents;

VehicleExtendedData<AdditionalComponents::VehicleComponents> AdditionalComponents::vehComps;
AdditionalComponents::eWiperState AdditionalComponents::m_currentWiperOneState = ONE_STATE_LEFT;
AdditionalComponents::eWiperState AdditionalComponents::m_currentWiperTwoState = TWO_STATE_LEFT;
AdditionalComponents::eOpenManholeState AdditionalComponents::m_currentManholeState = STATE_WAITING;
float AdditionalComponents::wiperOneAngle = 0.0f;
float AdditionalComponents::wiperTwoLAngle = 0.0f;
float AdditionalComponents::wiperTwoRAngle = 0.0f;
float AdditionalComponents::cementAngle = 0.0f;
float AdditionalComponents::manholeAngle = 0.0f;
