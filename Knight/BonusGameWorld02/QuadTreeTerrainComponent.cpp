#include "QuadTreeTerrainComponent.h"
#include "KnightUtils.h"

QuadTreeTerrainComponent::QuadTreeTerrainComponent()
{
    Type = Component::Model3D; // Set component type
	castShadow = Component::eShadowCastingType::NoShadow; // Default shadow casting type

}

QuadTreeTerrainComponent::~QuadTreeTerrainComponent()
{
    if (rootNode)
        delete rootNode; // Recursively deletes all nodes
    rootNode = nullptr;
    heightmap.clear(); // Clear heightmap data
    if (terrainTexture.id != 0) { // Unload texture if it was loaded
        UnloadTexture(terrainTexture);
    }
}

/// <summary>
/// CreateFromFile - Load heightmap and texture from files, initialize terrain and quadtree
/// </summary>
/// <param name="dimension">The world space coordinate of the dimension</param>
/// <param name="texTileSize">The tile size in the world space coordinate.</param>
/// <param name="pHightmapFilePath">The hightmap texture image path.</param>
/// <param name="pTerrainTexurePath">The default Terrain texture</param>
/// <returns></returns>
bool QuadTreeTerrainComponent::CreateFromFile(Vector3 dimension, Vector2 texTileSize, const char* pHightmapFilePath, const char* pTerrainTexurePath)
{
	// Load heightmap data from image.also set HeightMapWidth and terrainMapHeight
    if (!LoadHeightmapFromImage(pHightmapFilePath)) {
        return false;
    }

    //update terrain scale based on speficied terrain dimension
    terrainDimension = dimension;
    terrainScale.x = terrainDimension.x / HeightMapWidth; // X-axis scale
	terrainScale.z = terrainDimension.z / HeightMapDepth; // Z-axis scale
	terrainScale.y = terrainDimension.y; // Y-axis scale (from 0~1)

	//set texture tiling facotr based on the loaded heightmap
    tilingFactor.x = HeightMapWidth / texTileSize.x;  
	tilingFactor.y = HeightMapDepth / texTileSize.y;

    // Load terrain texture
    if (FileExists(pTerrainTexurePath)) {
        terrainTexture = LoadTextureForPlatform(pTerrainTexurePath);
        if (terrainTexture.id != 0) {
            SetTextureFilter(terrainTexture, TEXTURE_FILTER_TRILINEAR); // Optional: for smoother look
            SetTextureWrap(terrainTexture, TEXTURE_WRAP_REPEAT);
        }
    }
    else {
        terrainTexture.id = 0; // Ensure it's marked as invalid
    }

    // Define the overall bounds of the terrain in world space, using loaded dimensions
    BoundingBox terrainOverallBounds = {
        { -terrainDimension.x / 2.0f, -terrainScale.y * 0.1f, -terrainDimension.z / 2.0f }, // Min corner (allow small negative Y for visual bounding box)
        { terrainDimension.x / 2.0f, terrainScale.y * 1.1f, terrainDimension.z / 2.0f }     // Max corner (allow Y slightly above max height)
    };

    // Create and build the Quadtree root node
    rootNode = new QuadTreeNode(terrainOverallBounds, 0);
    BuildQuadtreeNode(rootNode);

    LocalBoundingBox = rootNode->bounds;

    //Create default material
	materials = new Material[1];
	materials[0] = LoadMaterialDefault(); // Load default material
	NumMaterial = 1; // Set number of materials

	return true;
}

/// <summary>
/// Update - Update component state, handle input for debugging
/// </summary>
/// <param name="ElapsedSeconds"></param>
/// <param name="pRH"></param>
void QuadTreeTerrainComponent::Update(float ElapsedSeconds, RenderHints* pRH)
{
	__super::Update(ElapsedSeconds, pRH); // Call base class update  

    if (IsKeyPressed(KEY_B)) {
        DebugShowBounds = !DebugShowBounds; // Toggle bounding box visibility
    }

}

/// <summary>
/// Draw - Render the terrain using quadtree for LOD and frustum culling
/// </summary>
/// <param name="pRH">The RenderHint data</param>
void QuadTreeTerrainComponent::Draw(RenderHints *pRH)
{
	__super::Draw(pRH); // Call base class draw

    FrustumPlane frustumPlanes[6]; // Array to hold the frustum planes
    NumTriangles = 0;

    nodesToDraw.clear(); // Clear previous frame's nodes to draw
    SceneCamera* pCam = _SceneActor->GetMainCamera();
    if (pRH != nullptr && pRH->pOverrideCamera != nullptr) {
        pCam = pRH->pOverrideCamera; // Use the override camera if provided
    }
    pCam->ExtractFrustumPlanes(frustumPlanes);
    GatherNodesToDraw(rootNode, pCam, frustumPlanes);

    if (pRH != nullptr && pRH->pOverrideShader != nullptr) {
		BeginShaderMode(*pRH->pOverrideShader); // Use the provided shader
        for (int i = 0; i < nodesToDraw.size(); i++) {
            DrawTerrainChunk(nodesToDraw[i]);
        }
		//printf("\n");
        //DrawQuadtreeNode(rootNode, _SceneActor->GetMainCamera(), DebugShowBounds, frustumPlanes);
		EndShaderMode(); // End shader mode
    }
    else 
    {
        //DrawQuadtreeNode(rootNode, _SceneActor->GetMainCamera(), DebugShowBounds, frustumPlanes); */
        for (int i = 0; i < nodesToDraw.size(); i++)
            DrawTerrainChunk(nodesToDraw[i]);
    }

}

// Get height from the global heightmap (with bounds checking)
float QuadTreeTerrainComponent::GetHeightmapValue(int x, int z)
{
    if (HeightMapDepth == 0 || HeightMapWidth == 0) return 0.0f; // No map loaded

    // Clamp coordinates to be within map boundaries
    if (x < 0) x = 0;
    if (x >= HeightMapWidth) x = HeightMapWidth - 1;
    if (z < 0) z = 0;
    if (z >= HeightMapDepth) z = HeightMapDepth - 1;

    return heightmap[z * HeightMapWidth + x];
}

//Get world position height 
float QuadTreeTerrainComponent::GetTerrainY(float x, float z)
{
	// Calculate the origin (bottom-left corner of the terrain)
	float worldOriginX = -terrainDimension.x / 2.0f;
	float worldOriginZ = -terrainDimension.z / 2.0f;
	// Convert to heightmap grid coordinates
	float presciseX = (x - worldOriginX) / terrainScale.x;
	float presciseZ = (z - worldOriginZ) / terrainScale.z;

    int mapX = (int)floor(presciseX);
	int mapZ = (int)floor(presciseZ);

	float h0 = GetHeightmapValue(mapX, mapZ); // Bottom-left corner height
	float h1 = GetHeightmapValue(mapX + 1, mapZ); // Bottom-right corner height
	float h2 = GetHeightmapValue(mapX, mapZ + 1); // Top-left corner height
	float h3 = GetHeightmapValue(mapX + 1, mapZ + 1); // Top-right corner height

    PointInTriangleResult result = DetectPointInTriangleAndCalcY(
        { (float)mapX, h0, (float)mapZ }, // Bottom-left
        { (float)mapX, h2, (float)mapZ + 1 }, // Bottom-right
        { (float)mapX + 1, h3, (float)mapZ + 1 }, // Top-left
        { presciseX, 0, presciseZ }); // Test point

    if (result.isInTriangle) 
    {
        // If the point is inside the triangle, return the calculated Y value
        return result.calculatedPy * terrainScale.y;
	} 

    result = DetectPointInTriangleAndCalcY(
        { (float)mapX, h0, (float)mapZ }, // Bottom-left
        { (float)mapX+1, h1, (float)mapZ}, // Bottom-right
        { (float)mapX + 1, h3, (float)mapZ + 1 }, // Top-left
        { presciseX, 0, presciseZ }); // Test point

	return result.calculatedPy * terrainScale.y; // Return the calculated Y value scaled by terrain scale

    //return (h0 * (1.0f - (presciseX - mapX)) * (1.0f - (presciseZ - mapZ)) + // Bottom-left
    //    h1 * (presciseX - mapX) * (1.0f - (presciseZ - mapZ)) + // Bottom-right
    //    h2 * (1.0f - (presciseX - mapX)) * (presciseZ - mapZ) + // Top-left
     //   h3 * (presciseX - mapX) * (presciseZ - mapZ)) * terrainScale.y; // Top-right

	// Get the height value from the heightmap
	//return GetHeightmapValue(mapX, mapZ) * terrainScale.y; // Scale by Y-axis terrain scale
}

/// <summary>
/// GetTerrainYForBoundingBox - Get the terrain height for a bounding box by sampling its corners and interpolating
/// </summary>
/// <param name="box">The BoundingBox</param>
/// <param name="weight">The interpolate weight to adjust returning height.</param>
/// <returns>The height of terrain in world space to place the BoundingBox on terrain.</returns>
float QuadTreeTerrainComponent::GetTerrainYForBoundingBox(BoundingBox box, float weight)
{
    Vector2 corners[4] = {
        { box.min.x, box.min.z }, // Bottom-left
        { box.max.x, box.min.z }, // Bottom-right
        { box.min.x, box.max.z }, // Top-left
        { box.max.x, box.max.z }  // Top-right
	};

    float maxh = -10000;
	float minh = 10000;
    for (int i = 0; i < 4; i++) {
        float result = GetTerrainY(corners[i].x, corners[i].y);
        if (maxh < result)
			maxh = result;
		if (minh > result)
			minh = result;
    }
	return Lerp(minh, maxh, weight); 
}

// Load heightmap data from a grayscale image
bool QuadTreeTerrainComponent::LoadHeightmapFromImage(const char* fileName)
{
    vector<unsigned int> outIndices;
	vector<Vector3> outVertices; 

    if (!FileExists(fileName)) {
        return false;
    }

    Image image = LoadImage(fileName);
    if (image.data == nullptr) { // Check if image loading failed
        return false;
    }

    // Store actual dimensions from the loaded image
    HeightMapWidth = image.width;
    HeightMapDepth = image.height;
    heightmap.resize(HeightMapWidth * HeightMapDepth);
    heightMapNormals.resize(HeightMapWidth * HeightMapDepth);

	outVertices.resize(HeightMapWidth * HeightMapDepth);

    //build height map
    for (int z = 0; z < HeightMapDepth; ++z) {
        for (int x = 0; x < HeightMapWidth; ++x) {
            Color pixelColor = GetImageColor(image, x, z);
            // For grayscale, R, G, and B components are usually the same.
            // We normalize the R component (0-255) to a float (0.0-1.0).
            heightmap[z * HeightMapWidth + x] = (float)pixelColor.r / 255.0f;
        }
    }

    // Build vertex positions
    heightMapNormals.assign(HeightMapWidth * HeightMapDepth, { 0,0,0 });

    for (int z = 0; z < HeightMapDepth; ++z) {
        for (int x = 0; x < HeightMapWidth; ++x) {
            float y = heightmap[z * HeightMapWidth + x];
            outVertices[z * HeightMapWidth + x] = { (float)x, y, (float)z };
        }
    }

    // To achieve smooth lighting for height - mapped terrain, the key concept is: using vertex normals. 
    // Instead of calculating normals per face(or per pixel), you calculate per - vertex normals by averaging normals of adjacent faces.
    // This ensures gradual lighting transitions, removing harsh shading.
    for (int z = 0; z < HeightMapDepth; ++z) {
        for (int x = 0; x < HeightMapWidth; ++x) {
            Vector3 normal = { 0, 0, 0 };

            Vector3 currentPos = {
                (x - HeightMapWidth / 2) * terrainScale.x,
                GetHeightmapValue(x, z) * terrainScale.y,
                (z - HeightMapDepth / 2) * terrainScale.z
            };

            // Check and average normals of surrounding triangles
            const int offsets[4][2] = { {-1,0}, {1,0}, {0,-1}, {0,1} };
            for (auto& offset : offsets) {
                int nx = x + offset[0];
                int nz = z + offset[1];

                if (nx < 0 || nz < 0 || nx >= HeightMapWidth || nz >= HeightMapDepth)
                    continue;

                Vector3 neighborPos = {
                    (nx - HeightMapWidth / 2) * terrainScale.x,
                    GetHeightmapValue(nx, nz) * terrainScale.y,
                    (nz - HeightMapDepth / 2) * terrainScale.z
                };

                Vector3 edge = Vector3Subtract(neighborPos, currentPos);

                int nx2 = x + offset[1];
                int nz2 = z - offset[0];

                if (nx2 < 0 || nz2 < 0 || nx2 >= HeightMapWidth || nz2 >= HeightMapDepth)
                    continue;

                Vector3 neighborPos2 = {
                    (nx2 - HeightMapWidth / 2) * terrainScale.x,
                    GetHeightmapValue(nx2, nz2) * terrainScale.y,
                    (nz2 - HeightMapDepth / 2) * terrainScale.z
                };

                Vector3 edge2 = Vector3Subtract(neighborPos2, currentPos);

                normal = Vector3Add(normal, Vector3CrossProduct(edge, edge2));
            }

            heightMapNormals[z * HeightMapWidth + x] = Vector3Normalize(normal);
        }
    }
    
    UnloadImage(image); // Free image data from RAM
    return true;
}

// Build the Quadtree recursively
void QuadTreeTerrainComponent::BuildQuadtreeNode(QuadTreeNode* node)
{
    // Stop subdividing if max depth is reached
    if (node->depth >= MaxQuadTreeDepth) {
        node->isLeaf = true;
        return;
    }

    // Calculate the size of potential children
    float halfSize = node->size / 2.0f;

    // Stop subdividing if the node's area is very small (e.g., covers less than one map pixel in extent)
    // This prevents creating too many nodes for tiny details.
    if (halfSize < terrainScale.x || halfSize < terrainScale.z) {
        node->isLeaf = true;
        return;
    }

    // If we are here, the node is not a leaf yet and can be subdivided
    node->isLeaf = false;

    // Get parent bounds
    Vector3 min = node->bounds.min;
    Vector3 max = node->bounds.max; // Y component of max is max terrain height

    // Define bounds for the four children
    // Child 0: Top-Left (NW)
    BoundingBox childBounds0 = { {min.x, min.y, min.z}, {min.x + halfSize, max.y, min.z + halfSize} };
    // Child 1: Top-Right (NE)
    BoundingBox childBounds1 = { {min.x + halfSize, min.y, min.z}, {max.x, max.y, min.z + halfSize} };
    // Child 2: Bottom-Left (SW)
    BoundingBox childBounds2 = { {min.x, min.y, min.z + halfSize}, {min.x + halfSize, max.y, max.z} };
    // Child 3: Bottom-Right (SE)
    BoundingBox childBounds3 = { {min.x + halfSize, min.y, min.z + halfSize}, {max.x, max.y, max.z} };

    node->children[0] = new QuadTreeNode(childBounds0, node->depth + 1);
    node->children[1] = new QuadTreeNode(childBounds1, node->depth + 1);
    node->children[2] = new QuadTreeNode(childBounds2, node->depth + 1);
    node->children[3] = new QuadTreeNode(childBounds3, node->depth + 1);

    // Recursively build children
    for (int i = 0; i < 4; ++i) {
        BuildQuadtreeNode(node->children[i]);
    }
}

/// <summary>
/// GetHeightmapNormal  - Get normal from the global heightmap (with bounds checking)
/// </summary>
/// <param name="x">x value</param>
/// <param name="y">z value</param>
/// <returns></returns>
Vector3 QuadTreeTerrainComponent::GetHeightmapNormal(int x, int y) 
{
    if (x < 0) x = 0;
    if (x >= HeightMapWidth) x = HeightMapWidth - 1;
    if (y < 0) y = 0;
    if (y >= HeightMapDepth) y = HeightMapDepth - 1;

    return heightMapNormals[y * HeightMapWidth + x];
}

Vector3 QuadTreeTerrainComponent::GetSmoothedNormal(float fx, float fz)
{
	int x = (int)(fx / terrainScale.x); // Convert world X to heightmap grid X
	int z = (int)(fz / terrainScale.z); // Convert world Z to heightmap grid Z

    // Bilinear interpolation for normals
    int x0 = std::min(x, HeightMapWidth - 1);
    int z0 = std::min(z, HeightMapDepth - 1);
    int x1 = std::min(x0 + 1, HeightMapWidth - 1);
    int z1 = std::min(z0 + 1, HeightMapDepth - 1);

    float tx = (float)(x - x0);
    float tz = (float)(z - z0);

    // Get normals at four surrounding points
    Vector3 n00 = heightMapNormals[z0* HeightMapWidth + x0];
    Vector3 n10 = heightMapNormals[z0* HeightMapWidth + x1];
    Vector3 n01 = heightMapNormals[z1 * HeightMapWidth + x0];
    Vector3 n11 = heightMapNormals[z1 * HeightMapWidth + x1];

    // Bilinear interpolation
    Vector3 n0 = Vector3Lerp(n00, n10, tx);
    Vector3 n1 = Vector3Lerp(n01, n11, tx);
    return Vector3Normalize(Vector3Lerp(n0, n1, tz));
}

// Draw the terrain chunk corresponding to a leaf node using immediate mode
void QuadTreeTerrainComponent::DrawTerrainChunk(QuadTreeNode* node)
{
    if (HeightMapWidth == 0 || HeightMapDepth == 0) return; // No map data
    if (terrainTexture.id == 0) return; // No texture loaded

    // Calculate the world origin (bottom-left corner of the terrain, assuming centered at 0,0)
    float worldOriginX = -terrainDimension.x / 2.0f;
    float worldOriginZ = -terrainDimension.z / 2.0f;

    // Convert node's world bounds to heightmap grid coordinates
    // Ensure we don't go outside the heightmap array
    int mapStartX = Clamp((int)roundf((node->bounds.min.x - worldOriginX) / terrainScale.x), 0, HeightMapWidth - 1);
    int mapStartZ = Clamp((int)roundf((node->bounds.min.z - worldOriginZ) / terrainScale.z), 0, HeightMapDepth - 1);
    int mapEndX = Clamp((int)roundf((node->bounds.max.x - worldOriginX) / terrainScale.x), 0, HeightMapWidth) +1;
    int mapEndZ = Clamp((int)roundf((node->bounds.max.z - worldOriginZ) / terrainScale.z), 0, HeightMapDepth) +1;

    // Ensure there's at least one quad to draw within the calculated range
    if (mapEndX <= mapStartX || mapEndZ <= mapStartZ)
        return;

    // Step determines the resolution of this chunk. For simplicity, step is 1.
    int step = 2;

    rlEnableTexture(terrainTexture.id); // Enable texturing

    rlBegin(RL_TRIANGLES); // Start drawing triangles
    rlColor4ub(255, 255, 255, 255); // Set vertex color to white to show original texture colors

    rlSetTexture(terrainTexture.id); // Bind the texture for drawing

    for (int z = mapStartZ; z < mapEndZ - step; z += step) { // Iterate through Z, stopping one step early for quad formation
        for (int x = mapStartX; x < mapEndX - step; x += step) { // Iterate through X, stopping one step early
            // Get normalized height values for the four corners of the current quad
            float h1 = GetHeightmapValue(x, z);             // Top-left
            float h2 = GetHeightmapValue(x + step, z);     // Top-right
            float h3 = GetHeightmapValue(x, z + step);     // Bottom-left
            float h4 = GetHeightmapValue(x + step, z + step); // Bottom-right

            Vector3 n1 = GetHeightmapNormal(x, z);             // Top-left
            Vector3 n2 = GetHeightmapNormal(x + step, z);     // Top-right
            Vector3 n3 = GetHeightmapNormal(x, z + step);     // Bottom-left
            Vector3 n4 = GetHeightmapNormal(x + step, z + step); // Bottom-right

            // Calculate world coordinates for the four corners, applying scaling
            Vector3 p1 = { worldOriginX + x * terrainScale.x ,             h1 * terrainScale.y, worldOriginZ + z * terrainScale.z };
            Vector3 p2 = { worldOriginX + (x + step) * terrainScale.x, h2 * terrainScale.y, worldOriginZ + z * terrainScale.z };
            Vector3 p3 = { worldOriginX + x * terrainScale.x ,             h3 * terrainScale.y, worldOriginZ + (z + step) * terrainScale.z };
            Vector3 p4 = { worldOriginX + (x + step) * terrainScale.x, h4 * terrainScale.y, worldOriginZ + (z + step) * terrainScale.z };

            // Calculate UV coordinates for each vertex of the quad
			// These map the texture across the entire terrain, tiled by tilingFactor
            float u1 = (float)x / (HeightMapWidth - 1.0f) * tilingFactor.x;
            float v1 = (float)z / (HeightMapDepth - 1.0f) * tilingFactor.y;

            float u2 = (float)(x + step) / (HeightMapWidth - 1.0f) * tilingFactor.x;
            float v2 = v1; // (float)z / (HeightMapDepth - 1.0f) * tilingFactor.y;

            float u3 = u1; // (float)x / (HeightMapWidth - 1.0f) * tilingFacotr.x;
            float v3 = (float)(z + step) / (HeightMapWidth - 1.0f) * tilingFactor.y;

            float u4 = u2; // (float)(x + step) / (HeightMapWidth - 1.0f) * tilingFactor.x;
            float v4 = v3; // (float)(z + step) / (HeightMapDepth - 1.0f) * tilingFactor.z;

			//New algorithm improvement: Use pre-calculated veretx normals for smooth lighting

            rlNormal3f(n1.x, n1.y, n1.z);  rlTexCoord2f(u1, v1); rlVertex3f(p1.x, p1.y, p1.z);  // p1
            rlNormal3f(n3.x, n3.y, n3.z);  rlTexCoord2f(u3, v3); rlVertex3f(p3.x, p3.y, p3.z); // p3
            rlNormal3f(n4.x, n4.y, n4.z);  rlTexCoord2f(u4, v4); rlVertex3f(p4.x, p4.y, p4.z); // p4

            rlNormal3f(n1.x, n1.y, n1.z);  rlTexCoord2f(u1, v1); rlVertex3f(p1.x, p1.y, p1.z); // p1
            rlNormal3f(n4.x, n4.y, n4.z);  rlTexCoord2f(u4, v4); rlVertex3f(p4.x, p4.y, p4.z); // p4
            rlNormal3f(n2.x, n2.y, n2.z);  rlTexCoord2f(u2, v2); rlVertex3f(p2.x, p2.y, p2.z); // p2

            /*
				//This is older version of the code that uses face normals for lighting
				//This sometimes is resulting inconsistent shaing result, not smooth lighting

                // Triangle 1: p1, p3, p4
                Vector3 nn1 = Vector3Normalize(Vector3CrossProduct(Vector3Subtract(p3, p1), Vector3Subtract(p4, p1)));
                rlNormal3f(nn1.x, nn1.y, nn1.z);

                rlTexCoord2f(u1, v1); rlVertex3f(p1.x, p1.y, p1.z); // p1
                rlTexCoord2f(u3, v3); rlVertex3f(p3.x, p3.y, p3.z); // p3
                rlTexCoord2f(u4, v4); rlVertex3f(p4.x, p4.y, p4.z); // p4

                // Triangle 2: p1, p4, p2
                Vector3 nn2 = Vector3Normalize(Vector3CrossProduct(Vector3Subtract(p4, p1), Vector3Subtract(p2, p1)));
                rlNormal3f(nn2.x, nn2.y, nn2.z);

                rlTexCoord2f(u1, v1); rlVertex3f(p1.x, p1.y, p1.z); // p1
                rlTexCoord2f(u4, v4); rlVertex3f(p4.x, p4.y, p4.z); // p4
                rlTexCoord2f(u2, v2); rlVertex3f(p2.x, p2.y, p2.z); // p2

            */

            NumTriangles += 2; // Increment triangle count (2 triangles per quad)
        }
    }
    rlEnd(); // Finish drawing triangles
    rlDisableTexture(); // Disable texturing
}

// Traverse the Quadtree and draw appropriate nodes/chunks
void QuadTreeTerrainComponent::DrawQuadtreeNode(QuadTreeNode* node, SceneCamera *pCamera, bool drawBounds, const FrustumPlane frustumPlanes[6])
{
    if (!node) return;

    // Frustum Culling: Check if the node's bounding box is visible
    if (!pCamera->IsBoundingBoxInFrustum(node->bounds, frustumPlanes)) {
        return; // Node is outside the frustum, so skip drawing it and its children
    }

	Camera3D camera = *pCamera->GetCamera3D(); // Get the camera from the SceneCamera component

    // Calculate distance from camera to node's center (on XZ plane)
    float dx = camera.position.x - node->center.x;
    float dz = camera.position.z - node->center.y; // node->center.y stores the Z-coordinate of the node's center
    float distanceToNode = sqrtf(dx * dx + dz * dz);

    // LOD Threshold: if distance is greater than node_size * factor, or if it's a leaf, or max depth draw it.
    // Otherwise, recurse into children.
    float lodThreshold = node->size * LevelOfDetailDistance;

    if (node->isLeaf || distanceToNode > lodThreshold || node->depth >= MaxQuadTreeDepth - 1) { // -1 to ensure leaves at max depth are drawn
        DrawTerrainChunk(node);

        // Optionally draw the bounding box for debugging
        if (drawBounds) {
            BoundingBox drawBox = node->bounds;
            // For visualization, we can set min/max Y of the box to actual terrain scale
            // A more accurate Y bound would involve sampling heights within this node.
            drawBox.min.y = -terrainScale.y * 0.05f; // Slightly below 0 in case of flat areas at 0 height
            drawBox.max.y = terrainScale.y * 1.05f;  // Slightly above max possible height
            Color boxColor = node->isLeaf ? LIME : RED; // Green for leaves, Red for branches
            DrawBoundingBox(drawBox, boxColor);
        }
    }
    else {
        // Recursively draw children (not a leaf and close enough to subdivide)
        for (int i = 0; i < 4; ++i) {
            if (node->children[i]) { // Check if child exists
                DrawQuadtreeNode(node->children[i], pCamera, drawBounds, frustumPlanes);
            }
        }
        // Optionally draw bounds of the parent node that got subdivided
        if (drawBounds) {
            DrawBoundingBox(node->bounds, YELLOW);
        }
    }
}

// Traverse the Quadtree and gather nodes to draw
void QuadTreeTerrainComponent::GatherNodesToDraw(QuadTreeNode* node, SceneCamera* pCamera, const FrustumPlane frustumPlanes[6])
{
    if (!node) return;

    // Frustum Culling: Check if the node's bounding box is visible
    if (pCamera->IsBoundingBoxInFrustum(node->bounds, frustumPlanes) == false) {
        return; // Node is outside the frustum, so skip drawing it and its children
    }

    Camera3D camera = *pCamera->GetCamera3D(); // Get the camera from the SceneCamera component

    // Calculate distance from camera to node's center (on XZ plane)
    float dx = camera.position.x - node->center.x;
    float dz = camera.position.z - node->center.y; // node->center.y stores the Z-coordinate of the node's center
    float distanceToNode = sqrtf(dx * dx + dz * dz);

    // LOD Threshold: if distance is greater than node_size * factor, or if it's a leaf, or max depth draw it.
    // Otherwise, recurse into children.
    float lodThreshold = node->size * LevelOfDetailDistance;

    if (node->isLeaf || distanceToNode > lodThreshold || node->depth >= MaxQuadTreeDepth - 1) { // -1 to ensure leaves at max depth are drawn
        //DrawTerrainChunk(node);
		nodesToDraw.push_back(node); // Add node to the list to draw
    }
    else {
        // Recursively draw children (not a leaf and close enough to subdivide)
        for (int i = 0; i < 4; ++i) {
            if (node->children[i]) { // Check if child exists
                GatherNodesToDraw(node->children[i], pCamera, frustumPlanes);
            }
        }

    }
}
