#include "plugin.h"
#include "CClumpModelInfo.h"
#include "CModelInfo.h"
#include "CWorld.h"
#include "extensions\KeyCheck.h"
#include "common.h"
#include <stdlib.h>
#include <vector>
#include <fstream>
#include <string>
#include "CStreaming.h"
#include "CTheScripts.h"

#include "CMessages.h"
#include "CFont.h"

CVector __cdecl PointOffset(CMatrix matrix, float x, float y, float z) {
    CVector pos;
    pos.x = matrix.pos.x + x * matrix.right.x + y * matrix.up.x + z * matrix.at.x;
    pos.y = matrix.pos.y + x * matrix.right.y + y * matrix.up.y + z * matrix.at.y;
    pos.z = matrix.pos.z + x * matrix.right.z + y * matrix.up.z + z * matrix.at.z;
    return pos;
}

float __cdecl Distance(CVector pos1, CVector pos2) {
    return (float)sqrt(pow(pos1.x - pos2.x, 2) + pow(pos1.y - pos2.y, 2) + pow(pos1.z - pos2.z, 2));
}

void MatrixSet(CMatrix *matrix, float cos_x, float sin_x, float cos_y, float sin_y, float cos_z, float sin_z) {
    matrix->right.x = cos_y * cos_z;
    matrix->right.y = cos_y * sin_z;
    matrix->right.z = -sin_y;
    matrix->up.x = sin_x * sin_y * cos_z - cos_x * sin_z;
    matrix->up.y = sin_x * sin_y * sin_z + cos_x * cos_z;
    matrix->up.z = sin_x * cos_y;
    matrix->at.x = cos_x * sin_y * cos_z + sin_x * sin_z;
    matrix->at.y = cos_x * sin_y * sin_z - sin_x * cos_z;
    matrix->at.z = cos_x * cos_y;
}

void MatrixAttach(CMatrix* matrix, CMatrix* rwMatrix, bool deleteOnDetach) {
    ((void(__thiscall *)(CMatrix*, CMatrix*, bool))0x4B8DD0)(matrix, rwMatrix, deleteOnDetach);
}

float VehicleGetSpeed(CVehicle *vehicle) {
    return 180.0f * (float)sqrt(pow(vehicle->m_vecMoveSpeed.x, 2) + pow(vehicle->m_vecMoveSpeed.y, 2) + pow(vehicle->m_vecMoveSpeed.z, 2));
}

using namespace plugin;
using namespace std;

class VehicleLodDist {
public:
    VehicleLodDist() {
        *(float *)0x600150 = 400.0f; // 70
        *(float *)0x60015C = 400.0f; // 60
    };
} vehicleLodDist;

class TruckTrailer {
public:
    class VehicleComponents {
    public:
        RwFrame *misc, *hookup, *prop_a, *prop_b, *prop_c;  char connector;
        CVehicle *m_pTractor, *m_pTrailer; bool enableSpeed;

        VehicleComponents(CVehicle *) { misc = hookup = prop_a = prop_b = prop_c = nullptr;  connector = 0; m_pTractor = m_pTrailer = nullptr; enableSpeed = true; }
    };

    static VehicleExtendedData<VehicleComponents> vehComps;

    class ModelInfo {
    public:
        bool enabledTrailer, isAttached;
        ModelInfo(CVehicle *vehicle) { enabledTrailer = true; isAttached = false; }
    };

    static VehicleExtendedData<ModelInfo> modelInfo;

    struct MyData {
        unsigned int ModelId, TrailerIdOne, TrailerIdTwo, TrailerIdThree, TrailerIdFour, TrailerColours, TrailerExtras, TrailerConst, TrailerSecond;
    };

    static vector<MyData>& GetDataVector() {
        static vector<MyData> vec;
        return vec;
    }

    static void ReadSettingsFile() {
        ifstream stream("trailer.dat");
        for (string line; getline(stream, line); ) {
            if (line[0] != ';' && line[0] != '#') {
                if (!line.compare("trailer")) {
                    while (getline(stream, line) && line.compare("end")) {
                        if (line[0] != ';' && line[0] != '#') {
                            MyData entry;
                            if (sscanf(line.c_str(), "%d, %d, %d, %d, %d, %d, %d, %d, %d", &entry.ModelId, &entry.TrailerIdOne, &entry.TrailerIdTwo, &entry.TrailerIdThree, &entry.TrailerIdFour, &entry.TrailerColours, &entry.TrailerExtras, &entry.TrailerConst, &entry.TrailerSecond) == 9)
                                GetDataVector().push_back(entry);
                        }
                    }
                }
            }
        }
    }

    static MyData *GetDataInfoForModel(unsigned int BaseModelId) {
        for (unsigned int i = 0; i < GetDataVector().size(); i++) {
            if (GetDataVector()[i].ModelId == BaseModelId)
                return &GetDataVector()[i];
        }
        return nullptr;
    }

    static void SetTrailer(CVehicle *vehicle, unsigned int modelTrailer, unsigned int colour, unsigned int extra, unsigned int enableSecondTrailer) {
        unsigned char oldFlags = CStreaming::ms_aInfoForModel[modelTrailer].m_nFlags;
        CStreaming::RequestModel(modelTrailer, GAME_REQUIRED);
        CStreaming::LoadAllRequestedModels(false);
        if (CStreaming::ms_aInfoForModel[modelTrailer].m_nLoadState == LOADSTATE_LOADED && vehicle) {
            //vehicle->m_autoPilot.m_nCruiseSpeed = 5.0f;
            //vehicle->m_nMoveFlags |= 1;
            //vehicle->m_fMass = vehicle->m_pHandlingData->m_fMass * 3.0f;
            //vehicle->m_fTurnMass = vehicle->m_pHandlingData->m_fTurnMass * 5.0f;
            if (extra) {
                CVehicleModelInfo::ms_compsToUse[0] = vehicle->m_anExtras[0];
                CVehicleModelInfo::ms_compsToUse[1] = vehicle->m_anExtras[1];
            }
            CVehicle *trailer = nullptr;
            trailer = new CAutomobile(modelTrailer, 1);
            if (trailer) {
                float offsetY = -4.0f;
                if (vehComps.Get(vehicle).misc && vehComps.Get(trailer).hookup)
                    offsetY = -((vehComps.Get(vehicle).misc->modelling.pos.y * (-1.0f)) + vehComps.Get(trailer).hookup->modelling.pos.y);
                trailer->SetPosition(vehicle->TransformFromObjectSpace(CVector(0.0f, offsetY, 0.0f)));
                trailer->SetHeading(vehicle->GetHeading() / 57.295776f);
                trailer->m_nState = 4;
                trailer->m_nDoorLock = CARLOCK_LOCKED;
                CWorld::Add(trailer);
                CTheScripts::ClearSpaceForMissionEntity(trailer->GetPosition(), trailer);
                reinterpret_cast<CAutomobile *>(trailer)->PlaceOnRoadProperly();
                if (colour) {
                    trailer->m_nPrimaryColor = vehicle->m_nPrimaryColor;
                    trailer->m_nSecondaryColor = vehicle->m_nSecondaryColor;
                }
                ModelInfo &infoTrailer = modelInfo.Get(trailer);
                infoTrailer.isAttached = true;
                // ������ ������
                int enable = plugin::Random(0, 2);
                if (enableSecondTrailer && vehComps.Get(trailer).misc && enable != 0) {
                    float offset_Y_Two = (-1.0f * (CModelInfo::ms_modelInfoPtrs[trailer->m_nModelIndex]->m_pColModel->m_boundBox.m_vecMin.y)) + CModelInfo::ms_modelInfoPtrs[trailer->m_nModelIndex]->m_pColModel->m_boundBox.m_vecMax.y;
                    CEntity *outEntityTwo;
                    short outCountTwo = 0;
                    CWorld::FindObjectsInRange(trailer->TransformFromObjectSpace(CVector(0.0f, -offset_Y_Two, 0.0f)), 2.0, 1, &outCountTwo, 2, &outEntityTwo, 0, 1, 0, 0, 0);
                    if (outCountTwo == 0) {
                        CVehicle *secondTrailer = nullptr;
                        secondTrailer = new CAutomobile(modelTrailer, 1);
                        if (secondTrailer) {
                            float offsetY_Two = -4.0f;
                            if (vehComps.Get(trailer).misc && vehComps.Get(secondTrailer).hookup)
                                offsetY_Two = -((vehComps.Get(trailer).misc->modelling.pos.y * (-1.0f)) + vehComps.Get(secondTrailer).hookup->modelling.pos.y);
                            secondTrailer->SetPosition(trailer->TransformFromObjectSpace(CVector(0.0f, offsetY_Two, 0.0f)));
                            secondTrailer->SetHeading(trailer->GetHeading() / 57.295776f);
                            secondTrailer->m_nState = 4;
                            secondTrailer->m_nDoorLock = CARLOCK_LOCKED;
                            CWorld::Add(secondTrailer);
                            CTheScripts::ClearSpaceForMissionEntity(secondTrailer->GetPosition(), secondTrailer);
                            reinterpret_cast<CAutomobile *>(secondTrailer)->PlaceOnRoadProperly();
                            if (colour) {
                                secondTrailer->m_nPrimaryColor = vehicle->m_nPrimaryColor;
                                secondTrailer->m_nSecondaryColor = vehicle->m_nSecondaryColor;
                            }
                            ModelInfo &infoTrailerTwo = modelInfo.Get(secondTrailer);
                            infoTrailerTwo.isAttached = true;
                        }
                    }
                }
            }
            if (!(oldFlags & GAME_REQUIRED)) {
                CStreaming::SetModelIsDeletable(modelTrailer);
                CStreaming::SetModelTxdIsDeletable(modelTrailer);
            }
        }
    }

    static void TrailerLightControl(CAutomobile *trailer) {
        CVehicleModelInfo *vehModel = reinterpret_cast<CVehicleModelInfo *>(CModelInfo::ms_modelInfoPtrs[trailer->m_nModelIndex]);
        if ((vehModel->m_avDummyPos[0].x == 0.0f) && (vehModel->m_avDummyPos[0].y == 0.0f) && (vehModel->m_avDummyPos[0].z == 0.0f)) {
            if (trailer->m_carDamage.GetLightStatus(LIGHT_FRONT_LEFT) == 0)
                trailer->m_carDamage.SetLightStatus(LIGHT_FRONT_LEFT, 1);
            if (trailer->m_carDamage.GetLightStatus(LIGHT_FRONT_RIGHT) == 0)
                trailer->m_carDamage.SetLightStatus(LIGHT_FRONT_RIGHT, 1);
        }
        if ((vehModel->m_avDummyPos[1].x == 0.0f) && (vehModel->m_avDummyPos[1].y == 0.0f) && (vehModel->m_avDummyPos[1].z == 0.0f)) {
            if (trailer->m_carDamage.GetLightStatus(LIGHT_REAR_LEFT) == 0)
                trailer->m_carDamage.SetLightStatus(LIGHT_REAR_LEFT, 1);
            if (trailer->m_carDamage.GetLightStatus(LIGHT_REAR_RIGHT) == 0)
                trailer->m_carDamage.SetLightStatus(LIGHT_REAR_RIGHT, 1);
        }
    }

    static bool TrailerAttached(CVehicle *trailer) {
        bool result = FALSE;
        for (int i = 0; i < CPools::ms_pVehiclePool->m_nSize; i++) {
            CVehicle *vehicle = CPools::ms_pVehiclePool->GetAt(i);
            if (vehicle && vehicle->m_nVehicleClass == VEHICLE_AUTOMOBILE && vehicle->m_fHealth > 0.1f && vehComps.Get(vehicle).misc
                && (vehComps.Get(vehicle).connector == vehComps.Get(trailer).connector)
                && (Distance(PointOffset(vehicle->m_matrix, 0, vehComps.Get(vehicle).misc->modelling.pos.y, vehComps.Get(vehicle).misc->modelling.pos.z), PointOffset(trailer->m_matrix, 0, vehComps.Get(trailer).hookup->modelling.pos.y, vehComps.Get(trailer).hookup->modelling.pos.z)) < 2.0f)) {
                result = TRUE; break;
            }
        }
        return result;
    }

    static void FrameSetRotateXOnly(RwFrame *component, float angle) {
        CMatrix matrixFrame(&component->modelling, false);
        matrixFrame.SetRotateXOnly(angle);
        matrixFrame.UpdateRW();
    }

    /*static CVector GetFramePosn(RwFrame *frame) {
        CVector posnFrame;
        RwV3d posn = RwFrameGetLTM(frame)->pos;
        posnFrame.x = posn.x;
        posnFrame.y = posn.y;
        posnFrame.z = posn.z;
        return posnFrame;
    }*/
    static CVector GetFramePosn(RwFrame *frame) {
        return RwFrameGetLTM(frame)->pos;
    }

    static void SearchTrailer(CVehicle *vehicle) {
        if (vehComps.Get(vehicle).misc) {
            for (auto trailer : CPools::ms_pVehiclePool) {
                if (vehComps.Get(trailer).hookup && (vehComps.Get(vehicle).connector == vehComps.Get(trailer).connector)) {
                    if (DistanceBetweenPoints(RwFrameGetLTM(vehComps.Get(vehicle).misc)->pos, RwFrameGetLTM(vehComps.Get(trailer).hookup)->pos) <= 2.0f) {
                        vehComps.Get(vehicle).m_pTrailer = trailer; break;
                    }
                }
            }
        }
    }


    TruckTrailer() {
        ReadSettingsFile();
        static unsigned int Id;
        static unsigned int TrailerId;
        static unsigned int currentVariant = 0;
        static CEntity *outEntity;
        static short outCount;

        Events::vehicleSetModelEvent += [](CVehicle *vehicle, int modelIndex) {
            if (vehicle->m_pRwClump) {
                vehComps.Get(vehicle).prop_a = CClumpModelInfo::GetFrameFromName(vehicle->m_pRwClump, "prop_a");
                vehComps.Get(vehicle).prop_b = CClumpModelInfo::GetFrameFromName(vehicle->m_pRwClump, "prop_b");
                vehComps.Get(vehicle).prop_c = CClumpModelInfo::GetFrameFromName(vehicle->m_pRwClump, "prop_c");
                vehComps.Get(vehicle).connector = 0;
                vehComps.Get(vehicle).misc = CClumpModelInfo::GetFrameFromName(vehicle->m_pRwClump, "misc_a");		if (vehComps.Get(vehicle).misc)	   vehComps.Get(vehicle).connector = 1;
                else {
                    vehComps.Get(vehicle).misc = CClumpModelInfo::GetFrameFromName(vehicle->m_pRwClump, "misc_b");		if (vehComps.Get(vehicle).misc) 	vehComps.Get(vehicle).connector = 2;
                    else {
                        vehComps.Get(vehicle).misc = CClumpModelInfo::GetFrameFromName(vehicle->m_pRwClump, "misc_c");		if (vehComps.Get(vehicle).misc) 	vehComps.Get(vehicle).connector = 3;
                        else {
                            vehComps.Get(vehicle).misc = CClumpModelInfo::GetFrameFromName(vehicle->m_pRwClump, "misc_d");		if (vehComps.Get(vehicle).misc) 	vehComps.Get(vehicle).connector = 4;
                            else {
                                vehComps.Get(vehicle).misc = CClumpModelInfo::GetFrameFromName(vehicle->m_pRwClump, "misc_e");		if (vehComps.Get(vehicle).misc) 	vehComps.Get(vehicle).connector = 5;
                                else {
                                    vehComps.Get(vehicle).misc = CClumpModelInfo::GetFrameFromName(vehicle->m_pRwClump, "misc_f");		if (vehComps.Get(vehicle).misc) 	vehComps.Get(vehicle).connector = 6;
                                }
                            }
                        }
                    }
                }
                vehComps.Get(vehicle).hookup = CClumpModelInfo::GetFrameFromName(vehicle->m_pRwClump, "hookup_a");	if (vehComps.Get(vehicle).hookup)		vehComps.Get(vehicle).connector = 1;
                else {
                    vehComps.Get(vehicle).hookup = CClumpModelInfo::GetFrameFromName(vehicle->m_pRwClump, "hookup_b");	if (vehComps.Get(vehicle).hookup)		vehComps.Get(vehicle).connector = 2;
                    else {
                        vehComps.Get(vehicle).hookup = CClumpModelInfo::GetFrameFromName(vehicle->m_pRwClump, "hookup_c");	if (vehComps.Get(vehicle).hookup)		vehComps.Get(vehicle).connector = 3;
                        else {
                            vehComps.Get(vehicle).hookup = CClumpModelInfo::GetFrameFromName(vehicle->m_pRwClump, "hookup_d");	if (vehComps.Get(vehicle).hookup)		vehComps.Get(vehicle).connector = 4;
                            else {
                                vehComps.Get(vehicle).hookup = CClumpModelInfo::GetFrameFromName(vehicle->m_pRwClump, "hookup_e");	if (vehComps.Get(vehicle).hookup)		vehComps.Get(vehicle).connector = 5;
                                else {
                                    vehComps.Get(vehicle).hookup = CClumpModelInfo::GetFrameFromName(vehicle->m_pRwClump, "hookup_f");	if (vehComps.Get(vehicle).hookup)		vehComps.Get(vehicle).connector = 6;
                                }
                            }
                        }
                    }
                }
            }
            else {
                vehComps.Get(vehicle).misc = vehComps.Get(vehicle).hookup = vehComps.Get(vehicle).prop_a = vehComps.Get(vehicle).prop_b = vehComps.Get(vehicle).prop_c = nullptr;
            }
        };

        Events::drawingEvent += [] {
            CPed *player = FindPlayerPed();
            if (player) {
                if (player->m_pVehicle && player->m_pVehicle->m_nVehicleClass == VEHICLE_AUTOMOBILE) {
                    vehComps.Get(player->m_pVehicle).m_pTrailer = nullptr;
                    if (player->m_bInVehicle) {
                        SearchTrailer(player->m_pVehicle);
                        if (vehComps.Get(player->m_pVehicle).m_pTrailer) {
                            if (vehComps.Get(player->m_pVehicle).enableSpeed) {
                                player->m_pVehicle->m_pHandlingData->m_transmissionData.m_fMaxGearVelocity *= 0.5f;
                                vehComps.Get(player->m_pVehicle).enableSpeed = false;
                            }
                        }
                        else if (!vehComps.Get(player->m_pVehicle).enableSpeed) {
                            player->m_pVehicle->m_pHandlingData->m_transmissionData.m_fMaxGearVelocity *= 2.0f;
                            vehComps.Get(player->m_pVehicle).enableSpeed = true;
                        }
                    }
                    else if (!vehComps.Get(player->m_pVehicle).enableSpeed) {
                        player->m_pVehicle->m_pHandlingData->m_transmissionData.m_fMaxGearVelocity *= 2.0f;
                        vehComps.Get(player->m_pVehicle).enableSpeed = true;
                    }
                    
                    gamefont::Print({
                        Format("enableSpeed %d", vehComps.Get(player->m_pVehicle).enableSpeed),
                        Format("MaxGearVelocity %.2f", player->m_pVehicle->m_pHandlingData->m_transmissionData.m_fMaxGearVelocity),
                        Format("ID trailer %d", vehComps.Get(player->m_pVehicle).m_pTrailer ? vehComps.Get(player->m_pVehicle).m_pTrailer->m_nModelIndex : -1)
                    }, 10, 10, 1, FONT_DEFAULT, 0.75f, 0.75f, color::Orange);
                    
                    /*CFont::SetScale(0.5f, 1.0f);
                    CFont::SetColor(CRGBA(238, 173, 53, 255));
                    CFont::SetJustifyOn();
                    CFont::SetFontStyle(0);
                    CFont::SetPropOn();
                    CFont::SetWrapx(600.0f);
                    wchar_t text[64];
                    swprintf(text, L"enableSpeed %d", vehComps.Get(player->m_pVehicle).enableSpeed);
                    CFont::PrintString(10.0f, 100.0f, text);
                    swprintf(text, L"MaxGearVelocity %.2f", player->m_pVehicle->m_pHandlingData->m_transmissionData.m_fMaxGearVelocity);
                    CFont::PrintString(10.0f, 120.0f, text);
                    if (vehComps.Get(player->m_pVehicle).m_pTrailer) {
                        swprintf(text, L"ID trailer %d", vehComps.Get(player->m_pVehicle).m_pTrailer->m_nModelIndex);
                        CFont::PrintString(10.0f, 80.0f, text);
                    }*/
                }
            }
        };

        Events::vehicleRenderEvent += [](CVehicle *vehicle) {
            CPed *player = FindPlayerPed();
            if (player) {
                //patch::SetFloat(0x6FAE24, 1.0f, true);  // camera
                for (int i = 0; i < CPools::ms_pVehiclePool->m_nSize; i++) {
                    CVehicle *trailer = CPools::ms_pVehiclePool->GetAt(i);
                    if (trailer && trailer->m_nVehicleClass == VEHICLE_AUTOMOBILE && trailer->m_fHealth > 0.1f) {
                        if (vehComps.Get(trailer).hookup) {
                            ModelInfo &infoTrailer = modelInfo.Get(trailer);
                            if (infoTrailer.isAttached && !trailer->GetIsOnScreen()) {
                                if (!TrailerAttached(trailer))
                                    CWorld::Remove(trailer);
                            }
                            trailer->m_nVehicleFlags.bEngineOn = 0;
                            if (vehComps.Get(trailer).prop_a)
                                vehComps.Get(trailer).prop_a->modelling.pos.z = -0.4f;
                            if (vehComps.Get(trailer).prop_b)
                                vehComps.Get(trailer).prop_b->modelling.pos.z = -0.2f;
                            if (vehComps.Get(trailer).prop_c)
                                FrameSetRotateXOnly(vehComps.Get(trailer).prop_c, 1.65f);
                            CAutomobile *trail = reinterpret_cast<CAutomobile *>(trailer);
                            for (int i = 0; i < CPools::ms_pVehiclePool->m_nSize; i++) {
                                CVehicle *vehicle = CPools::ms_pVehiclePool->GetAt(i);
                                if (vehicle && vehicle->m_nVehicleClass == VEHICLE_AUTOMOBILE && vehicle->m_fHealth > 0.1f && vehComps.Get(vehicle).misc) {
                                    CAutomobile *automobile = reinterpret_cast<CAutomobile *>(vehicle);
                                    if (vehicle && trailer && (vehComps.Get(vehicle).connector == vehComps.Get(trailer).connector) && (Distance(PointOffset(vehicle->m_matrix, 0, vehComps.Get(vehicle).misc->modelling.pos.y, vehComps.Get(vehicle).misc->modelling.pos.z), PointOffset(trailer->m_matrix, 0, vehComps.Get(trailer).hookup->modelling.pos.y, vehComps.Get(trailer).hookup->modelling.pos.z)) < 2.0f)) {
                                        if (player->m_pVehicle == vehicle) {
                                            //patch::SetFloat(0x6FAE24, 1.0f + 2 * CModelInfo::ms_modelInfoPtrs[trail->m_nModelIndex]->m_pColModel->m_boundBox.m_vecMin.y, true);	// camera
                                        }
                                        KeyCheck::Update();
                                        if ((KeyCheck::CheckWithDelay(VK_BACK, 200)) && player->m_pVehicle == vehicle && player->m_bInVehicle) {
                                            trailer->m_matrix.pos = PointOffset(trailer->m_matrix, 0, -2.1f, 0);
                                            trailer->m_nVehicleFlags.bEngineOn = 0;
                                        }
                                        else {
                                            if (vehComps.Get(trailer).prop_a)
                                                vehComps.Get(trailer).prop_a->modelling.pos.z = 0.0f;
                                            if (vehComps.Get(trailer).prop_b)
                                                vehComps.Get(trailer).prop_b->modelling.pos.z = 0.0f;
                                            if (vehComps.Get(trailer).prop_c)
                                                FrameSetRotateXOnly(vehComps.Get(trailer).prop_c, 0.0f);
                                            trailer->m_nVehicleFlags = vehicle->m_nVehicleFlags;
                                            //trailer->m_fBreakPedal = vehicle->m_fBreakPedal;
                                            //trailer->m_fGasPedal = vehicle->m_fGasPedal;
                                            TrailerLightControl(trail);
                                            bool  find;
                                            float LinkDifferenceZ = trailer->GetDistanceFromCentreOfMassToBaseOfModel() - vehicle->GetDistanceFromCentreOfMassToBaseOfModel();
                                            float TrailerOnGroundZ = min(0, CWorld::FindGroundZFor3DCoord(trailer->m_matrix.pos.x, trailer->m_matrix.pos.y, trailer->m_matrix.pos.z, &find) +
                                                trailer->GetDistanceFromCentreOfMassToBaseOfModel() - trailer->m_matrix.pos.z);
                                            if ((TrailerOnGroundZ < -1.0f) || (!VehicleGetSpeed(vehicle)))
                                                continue;
                                            CVector a = PointOffset(vehicle->m_matrix, 0, vehComps.Get(vehicle).misc->modelling.pos.y, LinkDifferenceZ);
                                            CVector b = PointOffset(trailer->m_matrix, 0, (trail)->m_aCarNodes[CAR_WHEEL_LB]->modelling.pos.y, TrailerOnGroundZ);
                                            float R = Distance(a, b);
                                            float cos_x = float(sqrt(pow(a.y - b.y, 2) + pow(b.x - a.x, 2)) / R);
                                            float sin_x = (a.z - b.z) / R;
                                            float cos_y = float(sqrt(pow(vehicle->m_matrix.right.x, 2) + pow(vehicle->m_matrix.right.y, 2)));
                                            float sin_y = -vehicle->m_matrix.right.z;
                                            float cos_z = (a.y - b.y) / (R * cos_x);
                                            float sin_z = (b.x - a.x) / (R * cos_x);
                                            CMatrix matrix;
                                            matrix.m_bOwnsAttachedMatrix = 0;
                                            matrix.m_pAttachMatrix = 0;
                                            MatrixAttach(&matrix, &trailer->m_matrix, 0);
                                            MatrixSet(&matrix, cos_x, sin_x, cos_y, sin_y, cos_z, sin_z);
                                            matrix.pos = a;
                                            matrix.pos = PointOffset(matrix, 0, -vehComps.Get(trailer).hookup->modelling.pos.y, 0);
                                            matrix.UpdateRW();
                                            matrix.~CMatrix();
                                            for (int w = 0; w < 4; w++)
                                                trail->m_fWheelTotalRot[w] = automobile->m_fWheelTotalRot[w];
                                            float vR = float(sqrt(pow(vehicle->m_vecMoveSpeed.x, 2) + pow(vehicle->m_vecMoveSpeed.y, 2)));
                                            float vx = -vR * sin_z;
                                            float vy = vR * cos_z;
                                            float vz = vehicle->m_vecMoveSpeed.z;
                                            trailer->m_vecMoveSpeed.x = vx;
                                            trailer->m_vecMoveSpeed.y = vy;
                                            trailer->m_vecMoveSpeed.z = vz;
                                        }
                                    }
                                }
                            }
                        }
                        // ����� ������� (trailer ����� ����)
                        else if (!CTheScripts::IsPlayerOnAMission() && trailer->m_pDriver && vehComps.Get(trailer).misc && player->m_pVehicle != trailer) {
                            MyData *entryModel = GetDataInfoForModel(trailer->m_nModelIndex);
                            ModelInfo &info = modelInfo.Get(trailer);
                            if (entryModel && info.enabledTrailer) {
                                if (!entryModel->TrailerConst) {
                                    if (currentVariant < 2)
                                        currentVariant += 1;
                                    else
                                        currentVariant = 0;
                                    if (currentVariant == 2)
                                        info.enabledTrailer = false;
                                }
                                switch (plugin::Random(0, 3)) {
                                case 0:  TrailerId = entryModel->TrailerIdOne;   break;
                                case 1:  TrailerId = entryModel->TrailerIdTwo;   break;
                                case 2:  TrailerId = entryModel->TrailerIdThree; break;
                                case 3:  TrailerId = entryModel->TrailerIdFour;  break;
                                default: TrailerId = entryModel->TrailerIdOne;   break;
                                }
                                if (info.enabledTrailer && (CModelInfo::IsVehicleModelType(TrailerId) == 0)) {
                                    float offset_Y = (-1.0f * (CModelInfo::ms_modelInfoPtrs[trailer->m_nModelIndex]->m_pColModel->m_boundBox.m_vecMin.y)) + CModelInfo::ms_modelInfoPtrs[TrailerId]->m_pColModel->m_boundBox.m_vecMax.y;
                                    outCount = 0;
                                    CWorld::FindObjectsInRange(trailer->TransformFromObjectSpace(CVector(0.0f, -offset_Y, 0.0f)), 2.0, 1, &outCount, 2, &outEntity, 0, 1, 0, 0, 0);
                                    if (outCount == 0) {
                                        info.enabledTrailer = false;
                                        if (DistanceBetweenPoints(player->GetPosition(), trailer->GetPosition()) > 50.0f)
                                            SetTrailer(trailer, TrailerId, entryModel->TrailerColours, entryModel->TrailerExtras, entryModel->TrailerSecond);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        };
    }
} truckTrailer;

VehicleExtendedData<TruckTrailer::VehicleComponents> TruckTrailer::vehComps;
VehicleExtendedData<TruckTrailer::ModelInfo> TruckTrailer::modelInfo;