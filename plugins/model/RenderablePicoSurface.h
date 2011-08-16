#ifndef RENDERABLEPICOSURFACE_H_
#define RENDERABLEPICOSURFACE_H_

#include "GLProgramAttributes.h"
#include "picomodel.h"
#include "render.h"
#include "math/AABB.h"

#include "ishaders.h"
#include "imodelsurface.h"

/* FORWARD DECLS */
class ModelSkin;
class RenderableCollector;
class SelectionTest;
class Selector;
class Shader;

namespace model
{

/* Renderable class containing a series of polygons textured with the same
 * material. RenderablePicoSurface objects are composited into a RenderablePicoModel
 * object to create a renderable static mesh.
 */

class RenderablePicoSurface :
	public IModelSurface,
	public OpenGLRenderable
{
	// Name of the material this surface is using, both originally and after a
	// skin remap.
	std::string _originalShaderName;
	std::string _mappedShaderName;

	// Shader object containing the material shader for this surface
	ShaderPtr _shader;

	// Vector of ArbitraryMeshVertex structures, containing the coordinates,
	// normals, tangents and texture coordinates of the component vertices
	typedef std::vector<ArbitraryMeshVertex> VertexVector;
	VertexVector _vertices;

	// Vector of render indices, representing the groups of vertices to be
	// used to create triangles
	typedef std::vector<unsigned int> Indices;
	Indices _indices;

	// Keep track of the number of indices to iterate over, since vector::size()
	// may not be fast
	unsigned int _nIndices;

	// The AABB containing this surface, in local object space.
	AABB _localAABB;

	// The GL display lists for this surface's geometry
	GLuint _dlRegular;
	GLuint _dlProgramPosVCol;
    GLuint _dlProgramNegVCol;
    GLuint _dlProgramNoVCol;

private:

	// Get a colour vector from an unsigned char array (may be NULL)
	Vector3 getColourVector(unsigned char* array);

	// Calculate tangent and bitangent vectors for all vertices.
	void calculateTangents();

	// Create the display lists
    GLuint compileProgramList(ShaderLayer::VertexColourMode);
	void createDisplayLists();

	std::string cleanupShaderName(const std::string& mapName);

public:
	/**
	 * Constructor. Accepts a picoSurface_t struct and the file extension to determine
	 * how to assign materials.
	 */
	RenderablePicoSurface(picoSurface_t* surf, const std::string& fExt);

	/**
	 * Destructor.
	 */
	~RenderablePicoSurface();

	/**
	 * Front-end render function used by the main renderer.
	 *
	 * @param rend
	 * The sorting RenderableCollector object which accepts renderable geometry.
	 *
	 * @param localToWorld
	 * Object to world-space transform.
	 *
	 * @param entity
	 * The entity this object is attached to.
	 */
	void submitRenderables(RenderableCollector& rend, const Matrix4& localToWorld,
						   const IRenderEntity& entity);

	/**
	 * Render function from OpenGLRenderable
	 */
	void render(const RenderInfo& info) const;

	/** Get the Shader for this surface.
	 */
	const ShaderPtr& getShader() const {
		return _shader;
	}

	/** Get the containing AABB for this surface.
	 */
	const AABB& getAABB() const {
		return _localAABB;
	}

	/** Apply the provided skin to this surface. If the skin has a remap for
	 * this surface's material, it will be applied, otherwise no action will
	 * occur.
	 *
	 * @param skin
	 * ModelSkin object to apply to this surface.
	 */
	void applySkin(const ModelSkin& skin);

	/**
	 * Perform a selection test on this surface.
	 */
	void testSelect(Selector& selector,
					SelectionTest& test,
					const Matrix4& localToWorld) const;

	// IModelSurface implementation
	int getNumVertices() const;
	int getNumTriangles() const;

	const ArbitraryMeshVertex& getVertex(int vertexIndex) const;
	ModelPolygon getPolygon(int polygonIndex) const;

	const std::string& getDefaultMaterial() const;
	const std::string& getActiveMaterial() const;
};

}

#endif /*RENDERABLEPICOSURFACE_H_*/
