/*
Plugin-SDK (Grand Theft Auto) source file
Authors: GTA Community. See more here
https://github.com/DK22Pac/plugin-sdk
Do not delete this comment block. Respect others' work!
*/
#include <plugin_III.h>
#include "game_III\CClumpModelInfo.h"

#include "game_III\common.h"
#include "game_III\CFont.h"

using namespace plugin;

class VehicleExtendedExample {
public:
    class VehicleComponents { // �����, ������� ������������ ���� ������ (����� �������, ��� ��� ������ "�������������" � ��������� ����������)
    public:
        RwFrame *m_pBootSliding;
        RwFrame *m_pSteerWheel;

        VehicleComponents(CVehicle *) { // ����������� ����� ������ ����� ������ ��� ������ ������������ ���������� (CVehicle::CVehicle)
            m_pBootSliding = m_pSteerWheel = nullptr;  // ������������� ��� ��������� � 0
        }
    };

    VehicleExtendedExample() {
        static VehicleExtendedData<VehicleComponents> vehComps; // ������� ��������� ������ ����������. vehComps - ��� ����������, ����� ������� �� �����
                                                                // ���������� � ����� ������ (��������� ����� Get(CVehicle *���������) )

        Events::vehicleSetModelEvent += [](CVehicle *vehicle, int modelIndex) { // ��������� ���� �������, ����� ���� ������������� ������ ����������
            if (vehicle->m_pRwClump) { // ���� ������ ����������� ������ ������ (RpClump)
                vehComps.Get(vehicle).m_pBootSliding = CClumpModelInfo::GetFrameFromName(vehicle->m_pRwClump, "boot_sliding"); // ������� ���������� � �������� � ���������� �� � ��� �����
                vehComps.Get(vehicle).m_pSteerWheel = CClumpModelInfo::GetFrameFromName(vehicle->m_pRwClump, "steerwheel"); 
                                                                                                                                
                
            }
            else  // ����� ������������ �� � 0
                vehComps.Get(vehicle).m_pBootSliding = vehComps.Get(vehicle).m_pSteerWheel = nullptr;
        };

        Events::drawingEvent += [] {
        //Events::gameProcessEvent += [] {
            for (int i = 0; i < CPools::ms_pVehiclePool->m_nSize; i++) {
                CVehicle *vehicle = CPools::ms_pVehiclePool->GetAt(i);
                if (vehicle && vehicle->GetIsOnScreen() && vehicle->m_nVehicleClass == VEHICLE_AUTOMOBILE) {
                    CAutomobile *automobile = reinterpret_cast<CAutomobile *>(vehicle);
                    // �������� ���������
                    if (vehComps.Get(vehicle).m_pBootSliding) {
                        if (automobile->m_aCarNodes[CAR_BOOT] && automobile->m_aCarNodes[CAR_WING_LR]) {
                            if (automobile->m_carDamage.GetPanelStatus(WING_REAR_LEFT) == 0) {
                                automobile->m_carDamage.SetDoorStatus(BOOT, 0);
                                vehComps.Get(vehicle).m_pBootSliding->modelling.at.z = 1.0f + automobile->m_aDoors[1].m_fAngle;
                            }
                            else 
                                automobile->m_carDamage.SetDoorStatus(BOOT, 3);
                        }
                    }
                    // ����
                    if (vehComps.Get(vehicle).m_pSteerWheel) {
                        CMatrix m(&vehComps.Get(vehicle).m_pSteerWheel->modelling, false);
                        m.SetRotateYOnly(vehicle->m_fSteerAngle * (-7.0f));
                        m.UpdateRW();
                    }
                    // 

                }
            }


            //CVehicle *vehicle = FindPlayerVehicle();
            //if (vehicle && vehicle->m_nVehicleClass == VEHICLE_AUTOMOBILE) {
            //    CAutomobile *automobile = reinterpret_cast<CAutomobile *>(vehicle);
            //    //if (automobile->m_aCarNodes[CAR_BOOT]) {
            //        // ����� ������
            //        CFont::SetScale(0.5f, 1.0f);
            //        CFont::SetColor(CRGBA(255, 255, 255, 255));
            //        CFont::SetJustifyOn();
            //        CFont::SetFontStyle(0);
            //        CFont::SetPropOn();
            //        CFont::SetWrapx(600.0f);
            //        wchar_t text[32];
            //        swprintf(text, L"steer angle %.2f", vehicle->m_fSteerAngle);
            //        CFont::PrintString(10.0f, 10.0f, text);
            //        
            //        /*swprintf(text, L"boot %d", automobile->m_carDamage.GetDoorStatus(BOOT));
            //        CFont::PrintString(10.0f, 10.0f, text);
            //        swprintf(text, L"wing %d", automobile->m_carDamage.GetPanelStatus(WING_REAR_LEFT));
            //        CFont::PrintString(10.0f, 30.0f, text);
            //        swprintf(text, L"angle %.2f", automobile->m_aDoors[1].m_fAngle);
            //        CFont::PrintString(10.0f, 50.0f, text);*/
            //    //}

            //}

        };

    }
} vehicleExtendedExample;
