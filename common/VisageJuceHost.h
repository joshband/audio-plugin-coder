/*
  ==============================================================================
    VisageJuceHost.h
    Bridge for JUCE 8 + Visage (Fixed Rendering Pipeline)
  ==============================================================================
*/
#pragma once
#include <JuceHeader.h>
#include "visage/app.h"
#include "visage/ui.h"
#include "visage/graphics.h"

// Crash Handler
static void npsCrashHandler(void*) {
    auto logFile = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
                   .getChildFile("APC_CRASH_REPORT.txt");
    juce::String report = "TIME: " + juce::Time::getCurrentTime().toString(true, true) + "\n";
    report += juce::SystemStats::getStackBacktrace();
    logFile.replaceWithText(report);
}

/**
 * VisagePluginEditor - A JUCE AudioProcessorEditor that hosts Visage UI
 * 
 * This class properly integrates Visage's rendering pipeline with JUCE's OpenGL context.
 * 
 * Key concepts:
 * 1. Visage uses a Frame hierarchy where each Frame has a Region
 * 2. The Canvas manages rendering and needs regions added to it
 * 3. Frames must be initialized and have their event handlers set up
 * 4. The redraw() mechanism triggers actual drawing via drawToRegion()
 */
class VisagePluginEditor : public juce::AudioProcessorEditor,
                           private juce::OpenGLRenderer
{
public:
    VisagePluginEditor(juce::AudioProcessor& p) : AudioProcessorEditor(&p) {
        static bool crashHandlerSet = false;
        if (!crashHandlerSet) {
            juce::SystemStats::setApplicationCrashHandler(npsCrashHandler);
            crashHandlerSet = true;
        }
        
        openGLContext.setOpenGLVersionRequired(juce::OpenGLContext::openGL3_2);
        openGLContext.setRenderer(this);
        
        // We do NOT want JUCE's software renderer to draw on top.
        openGLContext.setComponentPaintingEnabled(false); 
        
        openGLContext.setContinuousRepainting(true);
        openGLContext.attachTo(*this);
    }

    ~VisagePluginEditor() override {
        // Detach OpenGL context first - this will trigger openGLContextClosing()
        openGLContext.detach();
    }

    void paint(juce::Graphics& g) override { 
        // If you see RED, it means OpenGL is OFF and Software is ON.
        g.fillAll(juce::Colours::red); 
    }

    void resized() override { 
        onResize(getWidth(), getHeight()); 
    }

    void newOpenGLContextCreated() override {
        // Initialize Visage Renderer using JUCE's OpenGL context.
        void* nativeWindow = getPeer() ? getPeer()->getNativeHandle() : nullptr;
        void* glContext = openGLContext.getRawContext();
        void* backBuffer = reinterpret_cast<void*>(static_cast<uintptr_t>(openGLContext.getFrameBufferID()));
        
        // Initialize the renderer with the external OpenGL context
        visage::Renderer::instance().initialize(nativeWindow, nullptr, glContext, backBuffer);
        
        // Create the canvas
        canvas_ = std::make_unique<visage::Canvas>();
        
        // Get dimensions with DPI scaling
        const float scale = (float)openGLContext.getRenderingScale();
        const int w = juce::roundToInt(getWidth() * scale);
        const int h = juce::roundToInt(getHeight() * scale);
        
        // Pair canvas to the default back buffer (JUCE's FBO)
        canvas_->pairToDefaultBackBuffer(w, h);
        canvas_->setDpiScale(scale);
        
        // Set up the event handler for frame redraws
        // This is CRITICAL - without this, redraw() calls won't work
        eventHandler_.request_redraw = [this](visage::Frame* frame) {
            // Add frame to stale list for redrawing
            if (std::find(staleFrames_.begin(), staleFrames_.end(), frame) == staleFrames_.end())
                staleFrames_.push_back(frame);
        };
        
        eventHandler_.remove_from_hierarchy = [this](visage::Frame* frame) {
            auto pos = std::find(staleFrames_.begin(), staleFrames_.end(), frame);
            if (pos != staleFrames_.end())
                staleFrames_.erase(pos);
        };
        
        // Initialize the content frame
        rendererInitialized_ = true;
        onInit();
    }
    
    void renderOpenGL() override { 
        if (!rendererInitialized_ || !canvas_)
            return;
            
        // Handle High-DPI scaling
        const float scale = (float)openGLContext.getRenderingScale();
        const int w = juce::roundToInt(getWidth() * scale);
        const int h = juce::roundToInt(getHeight() * scale);
        
        // Update canvas dimensions if changed
        if (w != canvas_->width() || h != canvas_->height()) {
            canvas_->setDimensions(w, h);
            canvas_->setDpiScale(scale);
        }
        
        // Set OpenGL viewport
        juce::gl::glViewport(0, 0, w, h);

        // Clear to debug color - if you see this, Visage isn't drawing
        juce::OpenGLHelpers::clear(juce::Colours::magenta);

        // Let subclass prepare for render
        onRender();
        
        // Draw all stale frames
        drawStaleFrames();
        
        // Submit to GPU
        canvas_->submit();
    }
    
    void openGLContextClosing() override { 
        rendererInitialized_ = false;
        
        // Clear stale frames list
        staleFrames_.clear();
        
        // Let subclass clean up first
        onDestroy();
        
        // Remove canvas from window before destroying
        if (canvas_) {
            canvas_->removeFromWindow();
            canvas_.reset();
        }

        // Shutdown the renderer
        visage::Renderer::instance().shutdown();
    }

    // Override these in your subclass
    virtual void onInit() {}
    virtual void onRender() {}
    virtual void onDestroy() {}
    virtual void onResize(int w, int h) {}

protected:
    juce::OpenGLContext& getOpenGLContext() { return openGLContext; }
    visage::Canvas& getCanvas() { return *canvas_; }
    visage::FrameEventHandler& getEventHandler() { return eventHandler_; }
    
    /**
     * Add a frame to the canvas for rendering.
     * This sets up the frame's region and event handler.
     */
    void addFrameToCanvas(visage::Frame* frame) {
        if (!canvas_ || !frame)
            return;
            
        // Add the frame's region to the canvas
        canvas_->addRegion(frame->region());
        
        // Set up the event handler so redraw() works
        frame->setEventHandler(&eventHandler_);
        
        // Set DPI scale
        frame->setDpiScale((float)openGLContext.getRenderingScale());
        
        // Initialize the frame
        frame->init();
        
        // Trigger initial redraw
        frame->redrawAll();
    }
    
    /**
     * Remove a frame from the canvas.
     */
    void removeFrameFromCanvas(visage::Frame* frame) {
        if (!frame)
            return;
            
        // Clear event handler
        frame->setEventHandler(nullptr);
        
        // Remove from stale list
        auto pos = std::find(staleFrames_.begin(), staleFrames_.end(), frame);
        if (pos != staleFrames_.end())
            staleFrames_.erase(pos);
    }
    
    /**
     * Draw all frames that need redrawing.
     * This is called automatically in renderOpenGL().
     */
    void drawStaleFrames() {
        if (!canvas_)
            return;
            
        // Swap stale list to avoid issues if redraw() is called during draw
        std::vector<visage::Frame*> drawing;
        std::swap(staleFrames_, drawing);
        
        for (visage::Frame* frame : drawing) {
            if (frame && frame->isDrawing())
                frame->drawToRegion(*canvas_);
        }
        
        // Handle any frames that were added during drawing
        for (auto it = staleFrames_.begin(); it != staleFrames_.end();) {
            visage::Frame* frame = *it;
            if (std::find(drawing.begin(), drawing.end(), frame) == drawing.end()) {
                if (frame && frame->isDrawing())
                    frame->drawToRegion(*canvas_);
                it = staleFrames_.erase(it);
            } else {
                ++it;
            }
        }
    }

private:
    juce::OpenGLContext openGLContext;
    std::unique_ptr<visage::Canvas> canvas_;
    visage::FrameEventHandler eventHandler_;
    std::vector<visage::Frame*> staleFrames_;
    bool rendererInitialized_ = false;
};
