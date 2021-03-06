#pragma once

#include "irender.h"
#include <map>
#include "imodule.h"
#include "backend/OpenGLStateManager.h"
#include "backend/OpenGLShader.h"
#include "LinearLightList.h"
#include "render/backend/OpenGLStateLess.h"

namespace render
{

class GLProgramFactory;
typedef std::shared_ptr<GLProgramFactory> GLProgramFactoryPtr;

/**
 * \brief
 * Implementation of RenderSystem.
 */
class OpenGLRenderSystem
: public RenderSystem,
  public OpenGLStateManager,
  public ModuleObserver
{
private:
	// Map of named Shader objects
	typedef std::map<std::string, OpenGLShaderPtr> ShaderMap;
	ShaderMap _shaders;

	// whether this module has been realised
	bool _realised;

    // The GL program manager to acquire vfps
    GLProgramFactoryPtr _glProgramFactory;

    // Current shader program in use
    ShaderProgram _currentShaderProgram;

	// Map of OpenGLState references, with access functions.
	OpenGLStates _state_sorted;

	// Render time
	std::size_t _time;

	// Lights
	RendererLights m_lights;
	bool m_lightsChanged;
	typedef std::map<LitObject*, LinearLightList> LightLists;
	LightLists m_lightLists;

	sigc::signal<void> _sigExtensionsInitialised;

private:
	void propagateLightChangedFlagToAllLights();

public:

	/**
	 * Main constructor.
	 */
	OpenGLRenderSystem();

	virtual ~OpenGLRenderSystem();

    /* RenderSystem implementation */

	ShaderPtr capture(const std::string& name);
	void render(RenderStateFlags globalstate,
				const Matrix4& modelview,
				const Matrix4& projection,
				const Vector3& viewer);
	void realise();
	void unrealise();

    GLProgramFactory& getGLProgramFactory();

	std::size_t getTime() const;
	void setTime(std::size_t milliSeconds);

    ShaderProgram getCurrentShaderProgram() const;
    void setShaderProgram(ShaderProgram prog);

	void extensionsInitialised();
	sigc::signal<void> signal_extensionsInitialised();

	LightList& attachLitObject(LitObject& cullable);
	void detachLitObject(LitObject& cullable);
	void litObjectChanged(LitObject& cullable);

    // Attach and detach light sources
	void attachLight(RendererLight& light);
	void detachLight(RendererLight& light);
	void lightChanged(RendererLight& light);

	typedef std::set<const Renderable*> Renderables;
	Renderables m_renderables;
	mutable bool m_traverseRenderablesMutex;

    /* OpenGLStateManager implementation */
	void insertSortedState(const OpenGLStates::value_type& val) override;
	void eraseSortedState(const OpenGLStates::key_type& key) override;

	// renderables
	void attachRenderable(const Renderable& renderable);
	void detachRenderable(const Renderable& renderable);
	void forEachRenderable(const RenderableCallback& callback) const;

	// RegisterableModule implementation
    virtual const std::string& getName() const override;
    virtual const StringSet& getDependencies() const override;
    virtual void initialiseModule(const ApplicationContext& ctx) override;
    virtual void shutdownModule() override;
};
typedef std::shared_ptr<OpenGLRenderSystem> OpenGLRenderSystemPtr;

} // namespace render

