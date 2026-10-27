#include "BonusCoverflow3DUI.h"

// main entry function - the usual Knight app setting
int main()
{
    BonusCoverFlow3DUI* cf3dui = new BonusCoverFlow3DUI();

    cf3dui->Start();
    cf3dui->GameLoop();

    delete cf3dui;

    return 0;
}

//Initialize camera and cards during start up of the program
void BonusCoverFlow3DUI::Start()
{
    //Make sure _Scene is ready! We will at least add a Camera node
	__super::Start();  

    //Prepare a built-in camera and set its properties
    camera = _Scene->CreateSceneObject<PerspectiveCamera>("Camera");
    camera->SetPosition(Vector3{ 0.0f, 2.4f, 8.0f });
	camera->SetFovY(60.0f);
    camera->CameraMode = CameraMode::CAMERA_CUSTOM;
    camera->ShowCursor = false;
    camera->SetLookAtPosition(Vector3{ 0.0f, 1.0f,  0.0f });

    for (int i = 0; i < TOTAL_ITEMS; ++i)
    {
        ItemData data;
        data.id = i;
        data.baseColor = WHITE;

		// File path for this item's texture. In this case, all the poker card images.
        char pathBuf[128];
        snprintf(pathBuf, sizeof(pathBuf), (Config.ResourcesBasePath + "textures/PokerDeckCards/%d.png").c_str(), i % 52);
        data.texturePath = pathBuf;

        items.push_back(data);
    }

    for (int i = 0; i < VISIBLE_SLOTS; ++i)
    {
        slots[i].logicalIndex = -1;
        slots[i].offsetFromCenter = 0;
        slots[i].active = false;

        slots[i].card.texture.id = 0;
        slots[i].card.position = { 0.0f, 0.0f, 0.0f };
        slots[i].card.targetPosition = slots[i].card.position;
        slots[i].card.rotationY = 0.0f;
        slots[i].card.targetRotationY = 0.0f;
        slots[i].card.scale = 0.0f;
        slots[i].card.targetScale = 0.0f;
        slots[i].card.tint = WHITE;
    }

	//Initialize the visible cards based on the selected item index
    InitializeSlots();
}

//Update() is called every frame
void BonusCoverFlow3DUI::Update(float ElapsedSeconds)
{
	__super::Update(ElapsedSeconds);

    // 1. Generate ray from mouse position
    Ray ray = GetMouseRay(GetMousePosition(), *camera->GetCamera3D());

    int clickedLogicalIndex = -1;
    bool anyHovered = false;

    for (int i = 0; i < (int)slots.size(); ++i)
    {
        if (!slots[i].active) continue;

        Card& card = slots[i].card;

        // 2. Reset scale to default Coverflow state every frame
        // This ensures un-hovered cards shrink back to normal size
        ComputeCoverflowTransform(card, slots[i].offsetFromCenter);

        // 3. Calculate World Space Quad Vertices for Collision
        // We use current scale/pos for hit testing to match what the user sees
        float w = CARD_BASE_WIDTH * card.scale * 0.5f;
        float h = CARD_BASE_HEIGHT * card.scale * 0.5f;

        // Local corners (Top-Left, Bottom-Left, Bottom-Right, Top-Right)
        // Matches the vertex order used in DrawCardWithReflection
        Vector3 vTL = { -w,  h, 0 };
        Vector3 vBL = { -w, -h, 0 };
        Vector3 vBR = { w, -h, 0 };
        Vector3 vTR = { w,  h, 0 };

        // Rotate and Translate
        vTL = Vector3Add(RotateY(vTL, card.rotationY), card.position);
        vBL = Vector3Add(RotateY(vBL, card.rotationY), card.position);
        vBR = Vector3Add(RotateY(vBR, card.rotationY), card.position);
        vTR = Vector3Add(RotateY(vTR, card.rotationY), card.position);

        // 4. Check Collision
        RayCollision collision = GetRayCollisionQuad(ray, vTL, vBL, vBR, vTR);

        if (collision.hit) 
        {
            anyHovered = true;

            // APPLY HOVER EFFECT: Scale target up by 1.2x
            card.targetScale *= 1.2f;

            // CHECK CLICK
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) 
            {
                clickedLogicalIndex = slots[i].logicalIndex;
            }
        }
    }

    // Input Processing

    int diff = 0;
    bool selectionChanged = false;

    // Keyboard Input
    if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) 
        diff = 1;

    if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A))  
        diff = -1;

    // Mouse Input (Click)
    if (clickedLogicalIndex != -1 && clickedLogicalIndex != selectedIndex)
    {
        diff = clickedLogicalIndex - selectedIndex;
    }

	//Mouse wheel input
    float wheel = GetMouseWheelMove();

    // Check if the wheel moved significantly (e.g., > 0.0f)
    if (fabs(wheel) > 0.0f)
    {
        // Positive wheel movement (scroll up) moves selection LEFT (diff = -1)
        // Negative wheel movement (scroll down) moves selection RIGHT (diff = 1)
        if (wheel > 0)
        {
            diff = -1;
        }
        else if (wheel < 0)
        {
            diff = 1;
        }
    }

    // Apply Selection Change 

    if (diff != 0)
    {
        int newIndex = selectedIndex + diff;

        // Clamp bounds
        if (newIndex >= 0 && newIndex < TOTAL_ITEMS)
        {
            selectedIndex = newIndex;
            // Pass the calculated difference (direction)
            OnSelectionChanged(diff);
            selectionChanged = true;
        }
    }

    // Animate cards (lerp current -> target) 
    for (int i = 0; i < (int)slots.size(); ++i)
    {
        if (!slots[i].active) continue;
        Card& card = slots[i].card;

        card.position = Vector3Lerp(card.position, card.targetPosition, lerpSpeed);
        card.rotationY = Lerp(card.rotationY, card.targetRotationY, lerpSpeed);
        card.scale = Lerp(card.scale, card.targetScale, lerpSpeed);
    }

}

void BonusCoverFlow3DUI::DrawFrame()
{
	__super::DrawFrame();

    DrawGrid(40, 2.0f);  //a ground large enough 

    for (int i = 0; i < (int)slots.size(); ++i)
    {
        if (!slots[i].active) 
            continue;
        DrawCardWithReflection(slots[i].card);
    }

}

// Update HUD information
void BonusCoverFlow3DUI::DrawGUI()
{
    char buf[256];
    std::snprintf(buf, sizeof(buf),
        "Selected index: %d / %d  | Visible cards: %d  | Total items: %d  | Cache capacity: %zu",
        selectedIndex, TOTAL_ITEMS - 1, VISIBLE_SLOTS, TOTAL_ITEMS, CACHE_CAPACITY);

    DrawText("LEFT/RIGHT (or A/D) to move selection", 20, 20, 30, RAYWHITE);
    DrawText(buf, 20, 50, 30, YELLOW);

}

//Release resources
void BonusCoverFlow3DUI::EndGame()
{
    textureCache.UnloadAll();

	__super::EndGame();
}

void BonusCoverFlow3DUI::OnCreateDefaultResources()
{
    __super::OnCreateDefaultResources();
    //Loads a better TrueType font to display text information on the screen
    _Font = LoadFontEx((Config.ResourcesBasePath + "fonts/sparky.ttf").c_str(), 32, 0, 0);
}

//Handle selection change(with sliding)
// direction: +1 = move selection right, -1 = left
// direction (delta): +N = move selection right N steps, -N = left N steps

void BonusCoverFlow3DUI::OnSelectionChanged(int delta)
{
    const int radius = (int)slots.size() / 2;
    const int poolSize = (int)slots.size();

    // 1) Shift offsets based on the delta distance
    // If we move selection +3 (Right), cards must slide -3 (Left)
    for (int i = 0; i < poolSize; ++i)
    {
        slots[i].offsetFromCenter -= delta;
    }

    // 2) Wrap offsets so they remain within [-radius, +radius]
    // We use a while loop to handle jumps larger than the pool size
    for (int i = 0; i < poolSize; ++i)
    {
        VisualSlot& slot = slots[i];
        while (slot.offsetFromCenter < -radius) slot.offsetFromCenter += poolSize;
        while (slot.offsetFromCenter > +radius) slot.offsetFromCenter -= poolSize;
    }

    // 3) Recompute logical indices and update textures
    for (int i = 0; i < poolSize; ++i)
    {
        VisualSlot& slot = slots[i];
        Card& card = slot.card;

        int li = selectedIndex + slot.offsetFromCenter;

        if (li < 0 || li >= (int)items.size())
        {
            slot.active = false;
            slot.logicalIndex = -1;
            continue;
        }

        bool changed = (!slot.active) || (slot.logicalIndex != li);
        slot.active = true;
        slot.logicalIndex = li;

        if (changed)
        {
            card.texture = textureCache.Get(items[li].texturePath);
            card.tint = WHITE;
        }

        // Compute base target transform
        ComputeCoverflowTransform(card, slot.offsetFromCenter);
    }
}

// Coverflow layout for one card 
// offsetFromCenter = -VISIBLE_RADIUS .. +VISIBLE_RADIUS
void BonusCoverFlow3DUI::ComputeCoverflowTransform(Card& card, int offsetFromCenter)
{
    if (offsetFromCenter == 0)
    {
        card.targetPosition = { 0.0f, CARD_Y, BASE_Z + CENTER_Z_OFFSET };
        card.targetRotationY = 0.0f;
        card.targetScale = CENTER_SCALE * 1.10f;
    }
    else
    {
        float step = (float)std::abs(offsetFromCenter);
        float sign = (offsetFromCenter < 0) ? -1.0f : 1.0f;

        float x = sign * CARD_SPACING_X * step;
        float z = BASE_Z + Z_STEP * step + SIDE_EXTRA_Z * step;
        float rot = -SIDE_ANGLE * sign;
        float s = CENTER_SCALE * std::pow(SCALE_DROP, step);

        card.targetPosition = { x, CARD_Y, z };
        card.targetRotationY = rot;
        card.targetScale = s;
    }
}

void BonusCoverFlow3DUI::InitializeSlots()
{
    const int radius = (int)slots.size() / 2; // VISIBLE_RADIUS (9 -> 4)

    for (int i = 0; i < (int)slots.size(); ++i)
    {
        VisualSlot& slot = slots[i];

        slot.offsetFromCenter = i - radius;
        int li = selectedIndex + slot.offsetFromCenter;

        if (li < 0 || li >= (int)items.size())
        {
            slot.active = false;
            slot.logicalIndex = -1;
            slot.card.texture.id = 0;
            slot.card.position = { 0.0f, 0.0f, 0.0f };
            slot.card.targetPosition = slot.card.position;
            slot.card.rotationY = 0.0f;
            slot.card.targetRotationY = 0.0f;
            slot.card.scale = 0.0f;
            slot.card.targetScale = 0.0f;
            slot.card.tint = WHITE;
            continue;
        }

        slot.active = true;
        slot.logicalIndex = li;

        // Use LRU cache to load texture by file path
        slot.card.texture = textureCache.Get(items[li].texturePath); // CHANGED
        slot.card.tint = WHITE;

        // Set initial transform
        ComputeCoverflowTransform(slot.card, slot.offsetFromCenter);
        slot.card.position = slot.card.targetPosition;
        slot.card.rotationY = slot.card.targetRotationY;
        slot.card.scale = slot.card.targetScale;
    }
}

void BonusCoverFlow3DUI::DrawCardWithReflection(const Card& card)
{
    const float width = CARD_BASE_WIDTH * card.scale;
    const float height = CARD_BASE_HEIGHT * card.scale;
    const float half_w = width * 0.5f;
    const float half_h = height * 0.5f;

    // Main card
    rlPushMatrix();
    rlTranslatef(card.position.x, card.position.y, card.position.z);
    rlRotatef(card.rotationY, 0.0f, 1.0f, 0.0f);

    rlSetTexture(card.texture.id);
    rlColor4ub(255, 255, 255, 255);
    rlBegin(RL_QUADS);
    rlColor4ub(255, 255, 255, 255);

    rlTexCoord2f(0.0f, 0.0f); rlVertex3f(-half_w, half_h, 0.0f); // TR
    rlTexCoord2f(0.0f, 1.0f); rlVertex3f(-half_w, -half_h, 0.0f); // BR
    rlTexCoord2f(1.0f, 1.0f); rlVertex3f(half_w, -half_h, 0.0f); // BL
    rlTexCoord2f(1.0f, 0.0f); rlVertex3f(half_w, half_h, 0.0f); // TL

    rlEnd();
    rlSetTexture(0);
    rlPopMatrix();

    // Reflection
    float reflection_center_y = card.position.y - height;
    reflection_center_y += REFLECTION_OFFSET_Y;

    BeginBlendMode(BLEND_ALPHA);
    rlDisableDepthMask();

    rlPushMatrix();
    rlTranslatef(card.position.x, reflection_center_y, card.position.z);
    rlRotatef(card.rotationY, 0.0f, 1.0f, 0.0f);

    rlSetTexture(card.texture.id);
    rlBegin(RL_QUADS);

    const unsigned char alphaTop = static_cast<unsigned char>(255 * 0);   //reflection of "top side of the card" is fully transparent
    const unsigned char alphaBottom = static_cast<unsigned char>(255 * 0.5f);   //reflection fade start with half transparent, created a nice mirror effect

    rlColor4ub(255, 255, 255, alphaBottom);
    rlTexCoord2f(0.0f, 0.0f); rlVertex3f(-half_w, half_h, 0.0f);
    rlColor4ub(255, 255, 255, alphaTop);
    rlTexCoord2f(0.0f, 1.0f); rlVertex3f(-half_w, -half_h, 0.0f);
    rlColor4ub(255, 255, 255, alphaTop);
    rlTexCoord2f(1.0f, 1.0f); rlVertex3f(half_w, -half_h, 0.0f);
    rlColor4ub(255, 255, 255, alphaBottom);
    rlTexCoord2f(1.0f, 0.0f); rlVertex3f(half_w, half_h, 0.0f);

    rlEnd();
    rlSetTexture(0);
    rlPopMatrix();

    rlEnableDepthMask();
    EndBlendMode();
}

//End of BonusCoverflow3DUI.cpp