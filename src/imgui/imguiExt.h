#ifndef _IMGUI_EXT
#define _IMGUI_EXT

#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS 1
#endif
#include "imgui.h"
#include "imgui_internal.h"
#include "raylib.h"
#include <string>
#include <vector>

static inline ImVec4  operator*(const ImVec4& lhs, const float rhs)     { return ImVec4(lhs.x * rhs, lhs.y * rhs, lhs.z * rhs, lhs.w * rhs); }
static inline ImVec4  operator/(const ImVec4& lhs, const float rhs)     { return ImVec4(lhs.x / rhs, lhs.y / rhs, lhs.z / rhs, lhs.w / rhs); }

namespace ImGuiExt {
	void TextColored(const ImVec4& col, const char* text_start, const char* text_end = NULL);
	void PushTextColor(const ImVec4& col);
	void PopTextColor();

    bool BeginPopup(ImGuiID id, ImGuiPopupFlags flags = ImGuiPopupFlags_None);

    ImVec2 GetButtonSize(const char* str, const ImVec2& size_arg = {0,0});

	ImVec2 GetContentSize();
	void SetScrollNormX(float scrollAmt_norm, float centerRatio = 0.5f);
	void SetScrollNormY(float scrollAmt_norm, float centerRatio = 0.5f);
	float GetScrollBarHandleLen(ImGuiWindow* window, ImGuiAxis axis);
	ImRect GetScrollBarHandleRect(ImGuiWindow* window, ImGuiAxis axis);
	void AddLineToScrollBar(ImGuiWindow* window, ImGuiAxis axis, float pos_norm, const ImVec4& col, float thick = 1, ImRect scrollRect = {-1,-1,-1,-1});
	void AddRectToScrollBar(ImGuiWindow* window, ImGuiAxis axis, const ImRect& pos_norm, const ImVec4& col, ImRect scrollRect = {-1,-1,-1,-1});
	ImGuiLastItemData& GetItem();

    bool ToggleButton(const char* str_id, bool* v, const ImVec2& size = { 0,0 });
    template<typename T>
    bool SelectBits(const char* str_id, const char* const* labels, size_t num, T* selected, const ImVec2& size_arg = { 200,0 }) {
        ImGui::PushID(str_id);

        ImGui::BeginGroup();

        ImVec2 avail = ImGui::GetContentRegionAvail();
        ImVec2 size = ImVec2(size_arg.x != 0 ? size_arg.x : avail.x, size_arg.y != 0 ? size_arg.y : ImGui::GetFrameHeight());

        const ImVec2 borderPad = { 1,1 };
        ImRect rec(ImGui::GetCursorScreenPos() - borderPad, ImGui::GetCursorScreenPos() + size + borderPad);
        ImGui::GetWindowDrawList()->AddRectFilled(rec.Min, rec.Max, ImColor(ImGui::GetStyleColorVec4(ImGuiCol_ChildBg)));

        float CurrLineTextBaseOffset = ImGui::GetCurrentWindow()->DC.CurrLineTextBaseOffset;
        ImGui::GetCurrentWindow()->DC.CurrLineTextBaseOffset = 0;

        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0,ImGui::GetStyle().ItemSpacing.y });
        ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, {0.5,0.5});
        bool changed = false;
        for (size_t i = 0; i < num; i++) {
            ImGui::PushID((int)i);
            if (i > 0)
                ImGui::SameLine();

            size_t bitInd = num-i-1;

            const char* label;
            char buf[4];
            if(labels) {
                label = labels[i];
            }else{
                snprintf(buf, sizeof(buf), "%d", (int)bitInd);
                label = buf;
            }

            if (ImGui::Selectable(label, !!((*selected) & (1<<bitInd)), ImGuiSelectableFlags_NoPadWithHalfSpacing | ImGuiSelectableFlags_DontClosePopups, ImVec2(size.x / num, size.y))) {
                (*selected) ^= 1<<bitInd;
                changed = true;
            }
            ImGui::PopID();
        }
        ImGui::PopStyleVar(2);

        ImGui::GetWindowDrawList()->AddRect(rec.Min, rec.Max, ImColor(ImGui::GetStyleColorVec4(ImGuiCol_Border)));

        ImGui::GetCurrentWindow()->DC.CurrLineTextBaseOffset = CurrLineTextBaseOffset;

        ImGui::EndGroup();

        ImGui::PopID();
        return changed;
    }
	size_t SelectSwitch(const char* str_id, const char* const * labels, size_t num, size_t selected, const ImVec2& size = { 200,0 });
	void ImageRect(const Texture2D& tex, float destWidth, float destHeight, const Rectangle& srcRect);

	void Rect(const char* desc_id, const ImVec4& col, ImVec2 size = {0,0});
	void Rect(ImGuiID id, const ImVec4& col, ImVec2 size = {0,0});

	bool InputTextString(const char* label, const char* hint, std::string* str, ImGuiInputTextFlags flags = 0, const ImVec2& size = { 0,0 });

    bool Link(const char* str, const char* action_str = nullptr, std::function<void(const char*)> action = OpenURL);

    void Image(ImGuiID id, ImTextureID user_texture_id, const ImVec2& size, const ImVec2& uv0, const ImVec2& uv1, const ImVec4& tint_col = {1,1,1,1}, const ImVec4& border_col={0,0,0,0}, const ImVec2& pos={-INFINITY,-INFINITY}, ImDrawList* drawList = NULL);
	void ImageEx(ImGuiID id, ImTextureID user_texture_id, const ImVec2& size, const ImVec2& uv0, const ImVec2& uv1, const ImVec2& uv2, const ImVec2& uv3, const ImVec4& tint_col = {1,1,1,1}, const ImVec4& border_col={0,0,0,0}, const ImVec2& pos={-INFINITY,-INFINITY}, ImDrawList* drawList = NULL);
	void ImageRot90(ImGuiID id, ImTextureID user_texture_id, const ImVec2& size, uint8_t rotation, const ImVec2& uvMin = {0,0}, const ImVec2& uvMax = {1,1}, const ImVec4& tint_col = {1,1,1,1}, const ImVec4& border_col = {0,0,0,0}, const ImVec2& pos={-INFINITY,-INFINITY}, ImDrawList* drawList = NULL);

	ImVec4 BrightenColor(const ImVec4& col, float f);
    ImVec4 ColorFromInd(uint64_t ind, float saturation = .7f, float value = 1);

    // right align
    bool RA_Button(const char* str, const ImVec2& size_arg);
    void RightAlignText(const char* str, const char* str_end = 0);
}

#if defined(IMGUIEXT_IMPLEMENTATION)

void ImGuiExt::TextColored(const ImVec4& col, const char* text_start, const char* text_end) {
    ImGui::PushStyleColor(ImGuiCol_Text, col);
    ImGui::TextEx(text_start, text_end, ImGuiTextFlags_NoWidthForLargeClippedText); // Skip formatting
    ImGui::PopStyleColor();
}

void ImGuiExt::PushTextColor(const ImVec4& col) {
    ImGui::PushStyleColor(ImGuiCol_Text, col);
}

void ImGuiExt::PopTextColor() {
    ImGui::PopStyleColor();
}

bool ImGuiExt::BeginPopup(ImGuiID id, ImGuiWindowFlags flags) {
    ImGuiContext& g = *GImGui;
    if (g.OpenPopupStack.Size <= g.BeginPopupStack.Size) // Early out for performance
    {
        g.NextWindowData.ClearFlags(); // We behave like Begin() and need to consume those values
        return false;
    }
    flags |= ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings;
    return ImGui::BeginPopupEx(id, flags);
}

ImVec2 ImGuiExt::GetButtonSize(const char* str, const ImVec2& size_arg) {
    ImGuiStyle& style = ImGui::GetStyle();
    const ImVec2 label_size = ImGui::CalcTextSize(str, NULL, true);
    return ImGui::CalcItemSize(size_arg, label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);
}


ImVec2 ImGuiExt::GetContentSize() {
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    return { window->ContentSize.x + window->WindowPadding.x * 2.0f, window->ContentSize.y + window->WindowPadding.y * 2.0f };
}

void ImGuiExt::SetScrollNormX(float scrollAmt_norm, float centerRatio) {
    float contentWidth = ImGuiExt::GetContentSize().x;
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    window->ScrollTarget.y = contentWidth * scrollAmt_norm;
    window->ScrollTargetCenterRatio.y = centerRatio;
    window->ScrollTargetEdgeSnapDist.y = 0.0f;
}
void ImGuiExt::SetScrollNormY(float scrollAmt_norm, float centerRatio) {
    float contentHeight = ImGuiExt::GetContentSize().y;
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    window->ScrollTarget.y = contentHeight * scrollAmt_norm;
    window->ScrollTargetCenterRatio.y = centerRatio;
    window->ScrollTargetEdgeSnapDist.y = 0.0f;
}

float ImGuiExt::GetScrollBarHandleLen(ImGuiWindow* window, ImGuiAxis axis) {
    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;

    float size_avail = window->InnerRect.Max[axis] - window->InnerRect.Min[axis];
    float size_contents = window->ContentSize[axis] + window->WindowPadding[axis] * 2.0f;
    const float win_size = ImMax(ImMax(size_contents, size_avail), 1.0f);

    ImRect scrollBarRect = ImGui::GetWindowScrollbarRect(window, axis);
    float scrollBarWidth = axis == ImGuiAxis_X ? scrollBarRect.GetWidth() : scrollBarRect.GetHeight(); 

    return ImClamp(scrollBarWidth * (size_avail / win_size), style.GrabMinSize, scrollBarWidth);
}

ImRect ImGuiExt::GetScrollBarHandleRect(ImGuiWindow* window, ImGuiAxis axis){
    ImRect bb_frame = ImGui::GetWindowScrollbarRect(window,axis);
    float size_avail_v = window->InnerRect.Max[axis] - window->InnerRect.Min[axis];
    float size_contents_v = window->ContentSize[axis] + window->WindowPadding[axis] * 2.0f;
    const ImGuiStyle& style = ImGui::GetStyle();
    float* p_scroll_v = &window->Scroll[axis];

    const float bb_frame_width = bb_frame.GetWidth();
    const float bb_frame_height = bb_frame.GetHeight();
    if (bb_frame_width <= 0.0f || bb_frame_height <= 0.0f)
        return ImRect{{0,0},{0,0}};

    ImRect bb = bb_frame;
    bb.Expand(ImVec2(-ImClamp(IM_FLOOR((bb_frame_width - 2.0f) * 0.5f), 0.0f, 3.0f), -ImClamp(IM_FLOOR((bb_frame_height - 2.0f) * 0.5f), 0.0f, 3.0f)));
    
    // V denote the main, longer axis of the scrollbar (= height for a vertical scrollbar)
    const float scrollbar_size_v = (axis == ImGuiAxis_X) ? bb.GetWidth() : bb.GetHeight();

    // Calculate the height of our grabbable box. It generally represent the amount visible (vs the total scrollable amount)
    // But we maintain a minimum size in pixel to allow for the user to still aim inside.
    const float win_size_v = ImMax(ImMax(size_contents_v, size_avail_v), 1.0f);
    const float grab_h_pixels = ImClamp(scrollbar_size_v * (size_avail_v / win_size_v), style.GrabMinSize, scrollbar_size_v);
    
    float scroll_max = ImMax(1.0f, size_contents_v - size_avail_v);
    float scroll_ratio = ImSaturate(*p_scroll_v / scroll_max);
    float grab_v_norm = scroll_ratio * (scrollbar_size_v - grab_h_pixels) / scrollbar_size_v; // Grab position in normalized space

    ImRect grab_rect;
    if (axis == ImGuiAxis_X)
        grab_rect = ImRect(ImLerp(bb.Min.x, bb.Max.x, grab_v_norm), bb.Min.y, ImLerp(bb.Min.x, bb.Max.x, grab_v_norm) + grab_h_pixels, bb.Max.y);
    else
        grab_rect = ImRect(bb.Min.x, ImLerp(bb.Min.y, bb.Max.y, grab_v_norm), bb.Max.x, ImLerp(bb.Min.y, bb.Max.y, grab_v_norm) + grab_h_pixels);
    return grab_rect;
}

void ImGuiExt::AddLineToScrollBar(ImGuiWindow* window, ImGuiAxis axis, float pos_norm, const ImVec4& col, float thick, ImRect scrollRect) {
    if(scrollRect.Min.x == -1)
        scrollRect = ImGui::GetWindowScrollbarRect(window, axis);
    ImDrawList* drawList = window->DrawList;
    
    if (axis == ImGuiAxis_X) {
        float x = scrollRect.Min.x + (pos_norm * scrollRect.GetSize().x);
        drawList->AddLine(
            {x, scrollRect.Min.y},
            {x, scrollRect.Max.y},
            ImColor(col), thick
        );
    }
    else {
        float y = scrollRect.Min.y + (pos_norm * scrollRect.GetSize().y);
        drawList->AddLine(
            {scrollRect.Min.x, y},
            {scrollRect.Max.x, y},
            ImColor(col), thick
        );
    }
}

void ImGuiExt::AddRectToScrollBar(ImGuiWindow* window,  ImGuiAxis axis, const ImRect& pos_norm, const ImVec4& col, ImRect scrollRect) {
    if(scrollRect.Min.x == -1)
        scrollRect = ImGui::GetWindowScrollbarRect(window, axis);
    ImDrawList* drawList = window->DrawList;
    
    ImVec2 min = scrollRect.Min + (scrollRect.GetSize() * pos_norm.Min);
    ImVec2 max = min + (scrollRect.GetSize() * pos_norm.GetSize());
    drawList->AddRectFilled(min, max, ImColor(col));
}

ImGuiLastItemData& ImGuiExt::GetItem() {
    ImGuiContext& g = *GImGui;
    return g.LastItemData;
}

bool ImGuiExt::ToggleButton(const char* str_id, bool* v, const ImVec2& size_arg) {
    ImVec2 size = ImVec2(size_arg.x != 0 ? size_arg.x : ImGui::CalcTextSize(str_id).x + ImGui::GetStyle().FramePadding.x*2, size_arg.y != 0 ? size_arg.y : ImGui::GetFrameHeight());

    float CurrLineTextBaseOffset = ImGui::GetCurrentWindow()->DC.CurrLineTextBaseOffset;
    ImGui::GetCurrentWindow()->DC.CurrLineTextBaseOffset = 0;

    const ImVec2 borderPad = { 1,1 };
    ImRect rec(ImGui::GetCursorScreenPos() - borderPad, ImGui::GetCursorScreenPos() + size + borderPad);

    ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, {0.5,0.5});
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0,ImGui::GetStyle().ItemSpacing.y });

    bool changed = false;
    if (ImGui::Selectable(str_id, *v, ImGuiSelectableFlags_NoPadWithHalfSpacing, size)) {
        *v = !*v;
        changed = true;
    }

    ImGui::PopStyleVar(2);

    ImGui::GetCurrentWindow()->DC.CurrLineTextBaseOffset = CurrLineTextBaseOffset;

    ImGui::GetWindowDrawList()->AddRect(rec.Min, rec.Max, ImColor(ImGui::GetStyleColorVec4(ImGuiCol_Border)));

    return changed;
}
size_t ImGuiExt::SelectSwitch(const char* str_id, const char* const * labels, size_t num, size_t selected, const ImVec2& size_arg) {
    ImGui::PushID(str_id);

    ImGui::BeginGroup();

    ImVec2 avail = ImGui::GetContentRegionAvail();
    ImVec2 size = ImVec2(size_arg.x != 0 ? size_arg.x : avail.x, size_arg.y != 0 ? size_arg.y : ImGui::GetFrameHeight());

    const ImVec2 borderPad = { 1,1 };
    ImRect rec(ImGui::GetCursorScreenPos() - borderPad, ImGui::GetCursorScreenPos() + size + borderPad);
    ImGui::GetWindowDrawList()->AddRectFilled(rec.Min, rec.Max, ImColor(ImGui::GetStyleColorVec4(ImGuiCol_ChildBg)));

    float CurrLineTextBaseOffset = ImGui::GetCurrentWindow()->DC.CurrLineTextBaseOffset;
    ImGui::GetCurrentWindow()->DC.CurrLineTextBaseOffset = 0;

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0,ImGui::GetStyle().ItemSpacing.y });
    ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, {0.5,0.5});
    //bool changed = false;
    for (size_t i = 0; i < num; i++) {
        ImGui::PushID((int)i);
        if (i > 0)
            ImGui::SameLine();
        if (ImGui::Selectable(labels[i], selected == i, ImGuiSelectableFlags_NoPadWithHalfSpacing | ImGuiSelectableFlags_DontClosePopups, ImVec2(size.x / num, size.y))) {
            selected = i;
            //changed = true;
        }
        ImGui::PopID();
    }
    ImGui::PopStyleVar(2);

    ImGui::GetWindowDrawList()->AddRect(rec.Min, rec.Max, ImColor(ImGui::GetStyleColorVec4(ImGuiCol_Border)));

    ImGui::GetCurrentWindow()->DC.CurrLineTextBaseOffset = CurrLineTextBaseOffset;

    ImGui::EndGroup();

    ImGui::PopID();
    return selected;
}

void ImGuiExt::ImageRect(const Texture2D& tex, float destWidth, float destHeight, const Rectangle& srcRect) { // basically just copy-paste from rlImgui but with floats for dest dimensions
    ImVec2 uv0;
    ImVec2 uv1;

    if (srcRect.width < 0) {
        uv0.x = -((float)srcRect.x / tex.width);
        uv1.x = (uv0.x - (float)(fabs(srcRect.width) / tex.width));
    }
    else {
        uv0.x = (float)srcRect.x / tex.width;
        uv1.x = uv0.x + (float)(srcRect.width / tex.width);
    }

    if (srcRect.height < 0) {
        uv0.y = -((float)srcRect.y / tex.height);
        uv1.y = (uv0.y - (float)(fabs(srcRect.height) / tex.height));
    }
    else {
        uv0.y = (float)srcRect.y / tex.height;
        uv1.y = uv0.y + (float)(srcRect.height / tex.height);
    }

    ImGui::Image((ImTextureID*)&tex, ImVec2(float(destWidth), float(destHeight)),uv0,uv1);
}

void ImGuiExt::Rect(const char* desc_id, const ImVec4& col, ImVec2 size) {
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return;
    Rect(window->GetID(desc_id), col, size);
}

void ImGuiExt::Rect(ImGuiID id, const ImVec4& col, ImVec2 size) {
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return;

    ImGuiContext& g = *GImGui;
    float default_size = ImGui::GetFrameHeight();
    if (size.x == 0.0f)
        size.x = default_size;
    if (size.y == 0.0f)
        size.y = default_size;
    const ImRect bb(window->DC.CursorPos, ImVec2{ window->DC.CursorPos.x + size.x, window->DC.CursorPos.y + size.y });
    ImGui::ItemSize(bb, (size.y >= default_size) ? g.Style.FramePadding.y : 0.0f);
    if (!ImGui::ItemAdd(bb, id))
        return;

    ImGuiStyle& style = ImGui::GetStyle();
    ImGui::RenderNavHighlight(bb, id);
    ImGui::RenderFrame(bb.Min, bb.Max, ImColor(col), true, style.FrameRounding);
}


static int TextCallBack(ImGuiInputTextCallbackData* data) {
    std::string* str = (std::string*)data->UserData;

    if (data->EventFlag == ImGuiInputTextFlags_CallbackResize) {
        str->resize(data->BufTextLen+1);
        data->Buf = (char*)str->c_str();
    }
    return 0;
}
bool ImGuiExt::InputTextString(const char* label, const char* hint, std::string* str, ImGuiInputTextFlags flags, const ImVec2& size) {
    return ImGui::InputTextEx(
        label, hint, (char*)str->c_str(), (int)str->size() + 1, 
        size, flags | ImGuiInputTextFlags_CallbackResize, TextCallBack, str
    );
}

bool ImGuiExt::Link(const char* str, const char* action_str, std::function<void(const char*)> action) {
    constexpr ImVec4 linkColor = {.4f,.85f,1,1};
    ImGui::PushStyleColor(ImGuiCol_Text, linkColor);

    ImGui::TextUnformatted(str);
    if(ImGui::IsItemClicked())
        action(action_str ? action_str : str);

    ImRect r(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());

    ImGui::GetWindowDrawList()->AddLine(r.GetBL(), r.GetBR(), ImColor(linkColor));
    
    ImGui::PopStyleColor();

    return ImGui::IsItemClicked();
}


void ImGuiExt::Image(ImGuiID id, ImTextureID user_texture_id, const ImVec2& size, const ImVec2& uv0, const ImVec2& uv1, const ImVec4& tint_col, const ImVec4& border_col, const ImVec2& pos, ImDrawList* drawList) {
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return;

    ImVec2 pos_;
    if(isinf(pos.x) && isinf(pos.y)){
        pos_ = window->DC.CursorPos;
    }else{
        pos_ = pos;
    }
    
    if(drawList == nullptr)
        drawList = window->DrawList;

    ImRect bb(pos_, pos_ + size);
    if (border_col.w > 0.0f)
        bb.Max += ImVec2(2, 2);
    ImGui::ItemSize(bb);
    if (!ImGui::ItemAdd(bb, id))
        return;

    if (border_col.w > 0.0f)
    {
        drawList->AddRect(bb.Min, bb.Max, ImGui::GetColorU32(border_col), 0.0f);
        drawList->AddImage(user_texture_id, 
            bb.Min + ImVec2( 1, 1), bb.Max - ImVec2(1, 1), 
            uv0, uv1, 
            ImGui::GetColorU32(tint_col)
        );
    }
    else
    {
        drawList->AddImage(user_texture_id, 
            bb.Min, bb.Max, 
            uv0, uv1, 
            ImGui::GetColorU32(tint_col)
        );
    }
}

void ImGuiExt::ImageEx(ImGuiID id, ImTextureID user_texture_id, const ImVec2& size, const ImVec2& uv0, const ImVec2& uv1, const ImVec2& uv2, const ImVec2& uv3, const ImVec4& tint_col, const ImVec4& border_col, const ImVec2& pos, ImDrawList* drawList) {
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return;

    ImVec2 pos_;
    if(isinf(pos.x) && isinf(pos.y)){
        pos_ = window->DC.CursorPos;
    }else{
        pos_ = pos;
    }
    
    if(drawList == nullptr)
        drawList = window->DrawList;

    ImRect bb(pos_, pos_ + size);
    if (border_col.w > 0.0f)
        bb.Max += ImVec2(2, 2);
    ImGui::ItemSize(bb);
    if (!ImGui::ItemAdd(bb, id))
        return;

    if (border_col.w > 0.0f)
    {
        drawList->AddRect(bb.Min, bb.Max, ImGui::GetColorU32(border_col), 0.0f);
        drawList->AddImageQuad(user_texture_id, 
            bb.GetTL() + ImVec2( 1, 1), bb.GetTR() + ImVec2(-1, 1), bb.GetBR() + ImVec2(-1,-1), bb.GetBL() + ImVec2( 1,-1), 
            uv0, uv1, uv2, uv3, 
            ImGui::GetColorU32(tint_col)
        );
    }
    else
    {
        drawList->AddImageQuad(user_texture_id, 
            bb.GetTL(), bb.GetTR(), bb.GetBR(), bb.GetBL(), 
            uv0, uv1, uv2, uv3, 
            ImGui::GetColorU32(tint_col)
        );
    }
}

void ImGuiExt::ImageRot90(ImGuiID id, ImTextureID user_texture_id, const ImVec2& size, uint8_t rotation, const ImVec2& uvMin, const ImVec2& uvMax, const ImVec4& tint_col, const ImVec4& border_col, const ImVec2& pos, ImDrawList* drawList) {
    ImVec2 uv0,uv1,uv2,uv3;

    switch (rotation) {
    case 0:
        uv0 = { uvMin.x, uvMin.y };
        uv1 = { uvMax.x, uvMin.y };
        uv2 = { uvMax.x, uvMax.y };
        uv3 = { uvMin.x, uvMax.y };
        break;
    case 1:
        uv0 = { uvMin.x, uvMax.y };
        uv1 = { uvMin.x, uvMin.y };
        uv2 = { uvMax.x, uvMin.y };
        uv3 = { uvMax.x, uvMax.y };
        break;
    case 2:
        uv0 = { uvMax.x, uvMax.y };
        uv1 = { uvMin.x, uvMax.y };
        uv2 = { uvMin.x, uvMin.y };
        uv3 = { uvMax.x, uvMin.y };
        break;
    case 3:
        uv0 = { uvMax.x, uvMin.y };
        uv1 = { uvMax.x, uvMax.y };
        uv2 = { uvMin.x, uvMax.y };
        uv3 = { uvMin.x, uvMin.y };
        break;
    }

    ImGuiExt::ImageEx(id, user_texture_id, size, uv0, uv1, uv2, uv3, tint_col, border_col, pos, drawList);
}

ImVec4 ImGuiExt::BrightenColor(const ImVec4& col, float f) {
    return {col.x*f, col.y*f, col.z*f, col.w};
}
ImVec4 ImGuiExt::ColorFromInd(uint64_t ind, float saturation, float value) {
    ImVec4 col;
    ImGui::ColorConvertHSVtoRGB((float)(uint8_t)DataUtils::simpleHash(ind)/255, saturation, value, col.x, col.y, col.z);
    col.w = 1;
    return col;
}

void ImGuiExt::RightAlignText(const char* str, const char* str_end) {
    if (str_end == NULL)
        str_end = str + strlen(str);

    ImVec2 textSize = ImGui::CalcTextSize(str, str_end);
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + std::max(0.0f, ImGui::GetContentRegionAvail().x - textSize.x));
    ImGui::TextUnformatted(str, str_end);
}

bool ImGuiExt::RA_Button(const char* str, const ImVec2& size_arg) {
    const ImVec2 size = ImGuiExt::GetButtonSize(str, size_arg);
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + std::max(0.0f, ImGui::GetContentRegionAvail().x - size.x));
    return ImGui::Button(str, size_arg);
}


#endif

#endif