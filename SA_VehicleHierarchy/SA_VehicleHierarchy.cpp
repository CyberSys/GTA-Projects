#include <plugin.h>
#include "game_sa\common.h"
#include "game_sa\NodeName.h"
#include "game_sa\CFont.h"

using namespace plugin;

class VehicleHierarchy {
public:
    static RwFrame *DrawFrameHierarchyNames(RwFrame *frame, void *data) {
        CVector2D *offset = reinterpret_cast<CVector2D *>(data);
        // ������� ������� ��� ������ ������
        char *name = GetFrameNodeName(frame);
        CFont::PrintString(offset->x, offset->y, name[0] ? name : "<no_name>");
        offset->y += 20.0f; //���������� ����
                            // ������ ������� ����� ���� �������� �����������
        offset->x += 20.0f; // ����� ������� ������
        RwFrameForAllChildren(frame, DrawFrameHierarchyNames, offset); // ��� ���� �������� ������� �������� DrawFrameHierarchyNames. ����� �������, �� �������� ���� �����, ����� ���� �����, � �.�. ��� ���������� ���������.
        offset->x -= 20.0f; // ������ ���� �����
        return frame;
    }

    VehicleHierarchy() {
        Events::drawingEvent += [] {
            CVehicle *vehicle = FindPlayerVehicle(-1, false);
            if (vehicle && vehicle->m_pRwObject) {
                // ���������� ����� ������
                CFont::SetScale(0.5f, 1.0f);
                CFont::SetColor(CRGBA(255, 255, 255, 255));
                CFont::SetAlignment(ALIGN_LEFT);
                CFont::SetOutlinePosition(1);
                CFont::SetDropColor(CRGBA(0, 0, 0, 255));
                CFont::SetBackground(false, false);
                CFont::SetFontStyle(FONT_SUBTITLES);
                CFont::SetProp(true);
                CFont::SetWrapx(600.0f);
                // ������ �������, ������� � ��������� ������ (vehicle->m_pRwObject->parent) (���� "parent" ����� ��� void*, ������� �������� � RwFrame*.
                CVector2D offset = { 10.0f, 10.0f }; // ��������� ����������
                DrawFrameHierarchyNames(reinterpret_cast<RwFrame *>(vehicle->m_pRwObject->parent), &offset);
            }
        };
    }
} vehicleHierarchy;
