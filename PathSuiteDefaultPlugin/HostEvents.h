#pragma once
#include <string>
#include <stdexcept>
#include <memory>
#include "SpotPlugin.h"
#include "PluginHost.h"
#include "EventSourceTypes.h"
#include "EventArgConverters.h"

namespace HostInterop
{
    class HostEvents
    {
    public:
        typedef null_event_t                idle_event_t;
        typedef read_string_event_t         camera_initialize_t;
        typedef raw_event_t                 application_closing_t;
        typedef raw_event_t                 image_doc_changed_t;

        static HostEvents& Instance()
        {
            static HostEvents instance;
            return instance;
        }

        static idle_event_t& Idle()
        {
            if (nullptr == Instance().idleEventSource)
                Instance().idleEventSource = new idle_event_t(SpotPluginApi::HostEvent::Idle);
            return *(Instance().idleEventSource);
        }

        static camera_initialize_t& CameraInit()
        {
            if (nullptr == Instance().cameraInitEventSource)
                Instance().cameraInitEventSource = new camera_initialize_t(SpotPluginApi::HostEvent::CameraInitialized);
            return *(Instance().cameraInitEventSource);
        }

        static application_closing_t& ApplicationClosing()
        {
            if (nullptr == Instance().applicationClosingEventSource)
                Instance().applicationClosingEventSource = new application_closing_t(SpotPluginApi::HostEvent::ApplicationClosing);
            return *(Instance().applicationClosingEventSource);
        }

        static image_doc_changed_t& ImageDocChanged()
        {
            if (nullptr == Instance().imageDocChangedEventSource)
                Instance().imageDocChangedEventSource = new image_doc_changed_t(SpotPluginApi::HostEvent::ImageDocChanged);
            return *(Instance().imageDocChangedEventSource);
        }        

    private:

        // Private constructor because this is a singleton object. Use Instance() function for access to the object.
        HostEvents() :
            idleEventSource(nullptr),
            cameraInitEventSource(nullptr),
            applicationClosingEventSource(nullptr),
            imageDocChangedEventSource(nullptr)
        {
        }

        idle_event_t*               idleEventSource;
        camera_initialize_t*        cameraInitEventSource;
        application_closing_t*      applicationClosingEventSource;
        image_doc_changed_t*        imageDocChangedEventSource;
    };

}