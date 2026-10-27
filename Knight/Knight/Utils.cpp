#include "KnightUtils.h"

#include "rlgl.h"

#include <vector>
#include <unordered_map>
#include <config.h>
#include <algorithm> // Required for std::min and std::max

extern Rectangle& CenterRectangle(Rectangle& r, int width, int height)
{
    int w = DEFAULT_SCREEN_WIDTH;
	int h = DEFAULT_SCREEN_HEIGHT;

    if (Knight::Instance != nullptr) {
        w = Knight::Instance->Config.WindowWidth;
		h = Knight::Instance->Config.WindowHeight;
    }

	r.x = (float)(w - width) / 2;
	r.y = (float)(h - height) / 2;
	r.width = (float)width;
	r.height = (float)height;

	return r;
}

extern float DegreesToRadians(float degrees)
{
	return degrees * (3.14159265f / 180.0f);
}

// Function to calculate the 2D bounding rectangle of a 3D cube projected by Camera3D
extern BoundingRect Get2DBoundingRectOfCube(const Vector3& cubePosition, float cubeSize, const Camera3D& camera) {
    // Define the 8 corners of the cube
    std::vector<Vector3> cubeVertices = {
        { cubePosition.x - cubeSize / 2, cubePosition.y - cubeSize / 2, cubePosition.z - cubeSize / 2 },
        { cubePosition.x + cubeSize / 2, cubePosition.y - cubeSize / 2, cubePosition.z - cubeSize / 2 },
        { cubePosition.x - cubeSize / 2, cubePosition.y + cubeSize / 2, cubePosition.z - cubeSize / 2 },
        { cubePosition.x + cubeSize / 2, cubePosition.y + cubeSize / 2, cubePosition.z - cubeSize / 2 },
        { cubePosition.x - cubeSize / 2, cubePosition.y - cubeSize / 2, cubePosition.z + cubeSize / 2 },
        { cubePosition.x + cubeSize / 2, cubePosition.y - cubeSize / 2, cubePosition.z + cubeSize / 2 },
        { cubePosition.x - cubeSize / 2, cubePosition.y + cubeSize / 2, cubePosition.z + cubeSize / 2 },
        { cubePosition.x + cubeSize / 2, cubePosition.y + cubeSize / 2, cubePosition.z + cubeSize / 2 },
    };

    // Project each 3D vertex to 2D screen space
    std::vector<Vector2> projectedPoints;
    for (const auto& vertex : cubeVertices) {
        Vector2 screenPos = GetWorldToScreen(vertex, camera);
        projectedPoints.push_back(screenPos);
    }

    // Find the bounding rectangle in 2D screen space
    float minX = projectedPoints[0].x;
    float minY = projectedPoints[0].y;
    float maxX = projectedPoints[0].x;
    float maxY = projectedPoints[0].y;

    for (const auto& point : projectedPoints) {
        if (point.x < minX) minX = point.x;
        if (point.y < minY) minY = point.y;
        if (point.x > maxX) maxX = point.x;
        if (point.y > maxY) maxY = point.y;
    }

    return BoundingRect{ { minX, minY }, { maxX, maxY } };
}

extern void RecalculateSmoothNormals(const Model& model)
{
	for (int midx = 0; midx < model.meshCount; ++midx)
	{
		std::map<Vector3, Vector3, Vector3Compare> accumulatedNormals;

		Mesh& mesh = model.meshes[midx];

		// Ensure mesh data is present
		if (mesh.normals == nullptr)
		{
			mesh.normals = (float*)MemAlloc(mesh.vertexCount * 3 * sizeof(float));
			if (mesh.normals == nullptr)
			{
				TraceLog(LOG_ERROR, "Failed to allocate memory for mesh normals");
				return; // Memory allocation failed
			}
		}

		memset(mesh.normals, 0, mesh.vertexCount * 3 * sizeof(float));

		if (mesh.indices != nullptr)
		{
			// Accumulate normals per vertex
			for (int i = 0; i < mesh.triangleCount; ++i) {
				// Retrieve vertex indices
				unsigned short idx0 = mesh.indices[i * 3];
				unsigned short idx1 = mesh.indices[i * 3 + 1];
				unsigned short idx2 = mesh.indices[i * 3 + 2];

				// Retrieve vertex positions
				Vector3 v0 = {
					mesh.vertices[idx0 * 3],
					mesh.vertices[idx0 * 3 + 1],
					mesh.vertices[idx0 * 3 + 2]
				};
				Vector3 v1 = {
					mesh.vertices[idx1 * 3],
					mesh.vertices[idx1 * 3 + 1],
					mesh.vertices[idx1 * 3 + 2]
				};
				Vector3 v2 = {
					mesh.vertices[idx2 * 3],
					mesh.vertices[idx2 * 3 + 1],
					mesh.vertices[idx2 * 3 + 2]
				};

				// Compute face normal
				Vector3 edge1 = Vector3Subtract(v1, v0);
				Vector3 edge2 = Vector3Subtract(v2, v0);
				Vector3 faceNormal = Vector3Normalize(Vector3CrossProduct(edge1, edge2));

				accumulatedNormals[v0] = Vector3Add(accumulatedNormals[v0], faceNormal);
				accumulatedNormals[v1] = Vector3Add(accumulatedNormals[v1], faceNormal);
				accumulatedNormals[v2] = Vector3Add(accumulatedNormals[v2], faceNormal);
			}
		}
		else
		{
			for (int i = 0; i < mesh.vertexCount; i += 3) {
				Vector3 v0 = { mesh.vertices[i * 3], mesh.vertices[i * 3 + 1], mesh.vertices[i * 3 + 2] };
				Vector3 v1 = { mesh.vertices[(i + 1) * 3], mesh.vertices[(i + 1) * 3 + 1], mesh.vertices[(i + 1) * 3 + 2] };
				Vector3 v2 = { mesh.vertices[(i + 2) * 3], mesh.vertices[(i + 2) * 3 + 1], mesh.vertices[(i + 2) * 3 + 2] };

				Vector3 faceNormal = Vector3Normalize(Vector3CrossProduct(
					Vector3Subtract(v1, v0), Vector3Subtract(v2, v0)));

				accumulatedNormals[v0] = Vector3Add(accumulatedNormals[v0], faceNormal);
				accumulatedNormals[v1] = Vector3Add(accumulatedNormals[v1], faceNormal);
				accumulatedNormals[v2] = Vector3Add(accumulatedNormals[v2], faceNormal);
			}
		}

		for (int i = 0; i < mesh.vertexCount; ++i)
		{
			Vector3 pos = { mesh.vertices[i * 3], mesh.vertices[i * 3 + 1], mesh.vertices[i * 3 + 2] };
			Vector3 normal = Vector3Normalize(accumulatedNormals[pos]);
			mesh.normals[i * 3] = normal.x;
			mesh.normals[i * 3 + 1] = normal.y;
			mesh.normals[i * 3 + 2] = normal.z;
		}

		// Update mesh GPU data
		// in raylib, mesh buffers are updated using rlUpdateVertexBuffer
		// and normal buffer is usually at index 2
		UpdateMeshBuffer(mesh, 2, mesh.normals, mesh.vertexCount * 3 * sizeof(float), 0);
	}
}

struct Vertex {
    Vector3 position;
    Vector3 normal;
    Vector2 texcoord;

    bool operator==(const Vertex& other) const {
        return Vector3Equals(position, other.position);
    }
};

/// <summary>
/// VertexHash - Custom hash function for Vertex struct
/// </summary>
struct VertexHash {
    size_t operator()(const Vertex& v) const {
        size_t hx = std::hash<float>()(v.position.x);
        size_t hy = std::hash<float>()(v.position.y);
        size_t hz = std::hash<float>()(v.position.z);
        return hx ^ (hy << 1) ^ (hz << 2);
    }
};

extern void ConvertMeshToIndexed(Mesh* mesh)
{
    std::vector<Vertex> nonIndexedVertices;

    if (mesh->indices != nullptr) {
        TraceLog(LOG_WARNING, "Mesh is already indexed. No conversion needed. Skip this action.");
		return;
    }

	TraceLog(LOG_INFO, "Converting mesh to indexed format. Current vertices = %d", mesh->vertexCount);

    // Extract existing vertex data
    for (int i = 0; i < mesh->vertexCount; i++) {
        Vertex v = {};
        v.position = {
            mesh->vertices[i * 3 + 0],
            mesh->vertices[i * 3 + 1],
            mesh->vertices[i * 3 + 2]
        };

        if (mesh->normals) {
            v.normal = {
                mesh->normals[i * 3 + 0],
                mesh->normals[i * 3 + 1],
                mesh->normals[i * 3 + 2]
            };
        }

        if (mesh->texcoords) {
            v.texcoord = {
                mesh->texcoords[i * 2 + 0],
                mesh->texcoords[i * 2 + 1]
            };
        }

        nonIndexedVertices.push_back(v);
    }

    // Build unique vertices and index buffer
    std::vector<Vertex> uniqueVertices;
    std::vector<unsigned int> indices;
    std::unordered_map<Vertex, unsigned int, VertexHash> vertexToIndex;

    for (const auto& vertex : nonIndexedVertices) {
        auto it = vertexToIndex.find(vertex);
        if (it != vertexToIndex.end()) {
            indices.push_back(it->second);
        }
        else {
            unsigned int newIndex = static_cast<unsigned int>(uniqueVertices.size());
            uniqueVertices.push_back(vertex);
            vertexToIndex[vertex] = newIndex;
            indices.push_back(newIndex);
        }
    }

    // Free old CPU vertex arrays
    MemFree(mesh->vertices);
    if (mesh->normals) MemFree(mesh->normals);
    if (mesh->texcoords) MemFree(mesh->texcoords);
    if (mesh->indices) MemFree(mesh->indices);

    // Allocate new arrays
    mesh->vertexCount = (int)uniqueVertices.size();
    mesh->triangleCount = (int)indices.size() / 3;

    mesh->vertices = (float*)MemAlloc(mesh->vertexCount * 3 * sizeof(float));
    mesh->normals = (float*)MemAlloc(mesh->vertexCount * 3 * sizeof(float));
    mesh->texcoords = (float*)MemAlloc(mesh->vertexCount * 2 * sizeof(float));
    mesh->indices = (unsigned short*)MemAlloc((unsigned) indices.size() * sizeof(unsigned short));

    for (int i = 0; i < uniqueVertices.size(); i++) {
        mesh->vertices[i * 3 + 0] = uniqueVertices[i].position.x;
        mesh->vertices[i * 3 + 1] = uniqueVertices[i].position.y;
        mesh->vertices[i * 3 + 2] = uniqueVertices[i].position.z;

        mesh->normals[i * 3 + 0] = uniqueVertices[i].normal.x;
        mesh->normals[i * 3 + 1] = uniqueVertices[i].normal.y;
        mesh->normals[i * 3 + 2] = uniqueVertices[i].normal.z;

        mesh->texcoords[i * 2 + 0] = uniqueVertices[i].texcoord.x;
        mesh->texcoords[i * 2 + 1] = uniqueVertices[i].texcoord.y;
    }

    for (int i = 0; i < indices.size(); i++) {
        mesh->indices[i] = indices[i];
    }

    // Update counts
    mesh->triangleCount = (int)indices.size() / 3;

	//Check if the mesh has been uploaded to GPU
	//If Yes, we need to unload the mesh buffers and re-upload the mesh with new data
    if (mesh->vaoId > 0) {

        // Unload rlgl mesh vboId data
        // raylib will check if this mesh has been uploaded to GPU and skip the upload 
        // So we just unload the buffers here and then re-upload the mesh
        // Do not use UnloadMesh() as it will free the mesh structure itself
        rlUnloadVertexArray(mesh->vaoId);
        mesh->vaoId = 0;
        if (mesh->vboId != NULL)
            for (int i = 0; i < 9; i++) // raylib uses 9 VBO slots when GPU skinning is enabled
            {
                rlUnloadVertexBuffer(mesh->vboId[i]);
                mesh->vboId[i] = 0;
            }

        TraceLog(LOG_INFO, "Converting mesh to indexed format done. Current vertices = %d, indices=%d", mesh->vertexCount, mesh->triangleCount * 3);

        // Upload updated mesh to GPU (rebuilds VBOs/VAO)
        UploadMesh(mesh, true);

    }
}



// Function to check if a point (px, pz) is within the 2D projection of the triangle
// using barycentric coordinates (after projection).
// This is a common and robust 2D point-in-triangle test.
// We'll project to the XZ plane for this example.
bool IsPointInTriangle2D(Vector2 p, Vector2 v0, Vector2 v1, Vector2 v2) 
{
    // Vectors from v0 to other vertices
    Vector2 e1 = Vector2Subtract(v1, v0);
    Vector2 e2 = Vector2Subtract(v2, v0);

    // Vector from v0 to point p
    Vector2 p_minus_v0 = Vector2Subtract(p, v0);

    // Calculate dot products
    float dot00 = Vector2DotProduct(e1, e1);
    float dot01 = Vector2DotProduct(e1, e2);
    float dot02 = Vector2DotProduct(e1, p_minus_v0);
    float dot11 = Vector2DotProduct(e2, e2);
    float dot12 = Vector2DotProduct(e2, p_minus_v0);

    // Calculate barycentric coordinates
    float invDenom = 1.0f / (dot00 * dot11 - dot01 * dot01);
    float u = (dot11 * dot02 - dot01 * dot12) * invDenom;
    float v = (dot00 * dot12 - dot01 * dot02) * invDenom;

    // Check if point is in triangle
    return (u >= -EPSILON && v >= -EPSILON && (u + v) <= (1.0f + EPSILON));
}


PointInTriangleResult DetectPointInTriangleAndCalcY(
    Vector3 v0, Vector3 v1, Vector3 v2,
    Vector3 testPoint
) 
{
    PointInTriangleResult result = { false, std::numeric_limits<float>::quiet_NaN() };

    // 1. Calculate the plane equation (normal vector and D)
    Vector3 edge1 = Vector3Subtract(v1, v0);
    Vector3 edge2 = Vector3Subtract(v2, v0);

    Vector3 normal = Vector3Normalize(Vector3CrossProduct(edge1, edge2)); // Normalized normal vector

    // Equation of the plane: Ax + By + Cz + D = 0
    // D = -(normal.x * v0.x + normal.y * v0.y + normal.z * v0.z)
    float D = -Vector3DotProduct(normal, v0);

    // Check if the plane is degenerate (e.g., collinear vertices)
    if (Vector3LengthSqr(normal) < EPSILON * EPSILON) {
        TraceLog(LOG_WARNING, "Triangle is degenerate (collinear vertices). Cannot define plane.");
        return result;
    }

    // 2. Check if testPoint.xz lies within the 2D projection of the triangle
    // Project all points to 2D (XZ plane)
    Vector2 pv0 = { v0.x, v0.z };
    Vector2 pv1 = { v1.x, v1.z };
    Vector2 pv2 = { v2.x, v2.z };
    Vector2 p_xz = { testPoint.x, testPoint.z };

    if (!IsPointInTriangle2D(p_xz, pv0, pv1, pv2)) {
        // The 2D projection is not in the triangle, so the 3D point isn't either.
        return result;
    }

    // 3. Now that we know the projected point is inside the 2D triangle,
    //    calculate the Y value on the plane for the given (px, pz)
    // From Ax + By + Cz + D = 0, we solve for y:
    // By = -Ax - Cz - D
    // y = (-Ax - Cz - D) / B

    // Handle cases where B (normal.y) is very close to zero (triangle is vertical or near vertical)
    if (fabs(normal.y) < EPSILON) {
        // The plane is nearly vertical.
        // The point (px, pz) might be in the 2D projection, but there's no unique 'y' value for it on the plane,
        // or the triangle effectively has infinite height along Y at this XZ.
        // For game purposes, you might want to return false here, or handle it specifically.
        // E.g., if the triangle is perfectly vertical (normal.y == 0), then a line from (px, pz) upwards
        // will intersect the triangle, but there's no single y, rather a range of y values.
        TraceLog(LOG_INFO, "Triangle plane is vertical or near vertical (normal.y close to zero). Cannot determine unique Y.");
        // We still return true for 'isInTriangle' if it passed the 2D test, but 'calculatedPy' will be NaN
        result.isInTriangle = true; // It's on the plane, and XZ is within bounds
        return result;
    }

    result.calculatedPy = (-normal.x * testPoint.x - normal.z * testPoint.z - D) / normal.y;
    result.isInTriangle = true;

    return result;
}

// Helper: compare two Vector3 with tolerance (floating point safety)
bool Vector3AlmostEquals(const Vector3& a, const Vector3& b, float epsilon = 1e-6f) 
{
    return (fabs(a.x - b.x) < epsilon) &&
        (fabs(a.y - b.y) < epsilon) &&
        (fabs(a.z - b.z) < epsilon);
}

// Compare two Camera3D settings
bool Camera3DEquals(const Camera3D& a, const Camera3D& b, float epsilon) 
{
    return Vector3AlmostEquals(a.position, b.position, epsilon) &&
        Vector3AlmostEquals(a.target, b.target, epsilon) &&
        Vector3AlmostEquals(a.up, b.up, epsilon) &&
        fabs(a.fovy - b.fovy) < epsilon &&
        a.projection == b.projection;
}

BoundingBox GetBoundingBoxUnion(BoundingBox box1, BoundingBox box2) 
{
    BoundingBox result = { 0 };

    result.min.x = std::min(box1.min.x, box2.min.x);
    result.min.y = std::min(box1.min.y, box2.min.y);
    result.min.z = std::min(box1.min.z, box2.min.z);

    result.max.x = std::max(box1.max.x, box2.max.x);
    result.max.y = std::max(box1.max.y, box2.max.y);
    result.max.z = std::max(box1.max.z, box2.max.z);

    return result;
}

/// <summary>
/// PointToBoxDistanceSqr - calculate minimum squared distance between point and BoundingBox
/// </summary>
/// <param name="p">Vector3 point in the coordinate space</param>
/// <param name="box">BoundingBox in the same coordinate space</param>
/// <returns>The square of nearest distance from point to the BoundingBox</returns>
extern float PointToBoxDistanceSqr(Vector3 p, BoundingBox box)
{
    float dx = 0.0f;
    if (p.x < box.min.x) dx = box.min.x - p.x;
    else if (p.x > box.max.x) dx = p.x - box.max.x;

    float dy = 0.0f;
    if (p.y < box.min.y) dy = box.min.y - p.y;
    else if (p.y > box.max.y) dy = p.y - box.max.y;

    float dz = 0.0f;
    if (p.z < box.min.z) dz = box.min.z - p.z;
    else if (p.z > box.max.z) dz = p.z - box.max.z;

    return dx * dx + dy * dy + dz * dz;
}

/// <summary>
/// ValidBoundingBox - determine if a BoundingBox is valid (min < max on z axes)
/// </summary>
/// <param name="box">BoundingBox</param>
/// <returns>true if the BoundingBox is valid</returns>
extern bool IsBoundingBoxValid(BoundingBox box)
{
	if (box.min.z >= box.max.z) return false;
    return true;
}

//end of Utils.cpp
