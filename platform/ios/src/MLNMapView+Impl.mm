#import "MLNMapView+Impl.h"
#import "MLNMapView_Private.h"
#import "MLNStyle_Private.h"
#import "NSBundle+MLNAdditions.h"

#if MLN_RENDER_BACKEND_METAL
#import "MLNMapView+Metal.h"
#else // MLN_RENDER_BACKEND_OPENGL
#import "MLNMapView+OpenGL.h"
#endif

std::unique_ptr<MLNMapViewImpl> MLNMapViewImpl::Create(MLNMapView* nativeView) {
#if MLN_RENDER_BACKEND_METAL
    return std::make_unique<MLNMapViewMetalImpl>(nativeView);
#else // MLN_RENDER_BACKEND_OPENGL
    return std::make_unique<MLNMapViewOpenGLImpl>(nativeView);
#endif
}

MLNMapViewImpl::MLNMapViewImpl(MLNMapView* nativeView_) : mapView(nativeView_) {
}

void MLNMapViewImpl::render() {
    [mapView renderSync];
}

void MLNMapViewImpl::onCameraWillChange(mbgl::MapObserver::CameraChangeMode mode) {
    bool animated = mode == mbgl::MapObserver::CameraChangeMode::Animated;
    [mapView cameraWillChangeAnimated:animated];
}

void MLNMapViewImpl::onCameraIsChanging() {
    [mapView cameraIsChanging];
}

void MLNMapViewImpl::onCameraDidChange(mbgl::MapObserver::CameraChangeMode mode) {
    bool animated = mode == mbgl::MapObserver::CameraChangeMode::Animated;
    [mapView cameraDidChangeAnimated:animated];
}

void MLNMapViewImpl::onWillStartLoadingMap() {
    [mapView mapViewWillStartLoadingMap];
}

void MLNMapViewImpl::onDidFinishLoadingMap() {
    [mapView mapViewDidFinishLoadingMap];
}

void MLNMapViewImpl::onDidFailLoadingMap(mbgl::MapLoadError mapError, const std::string& what) {
    NSString *description;
    MLNErrorCode code;
    switch (mapError) {
        case mbgl::MapLoadError::StyleParseError:
            code = MLNErrorCodeParseStyleFailed;
            description = NSLocalizedStringWithDefaultValue(@"PARSE_STYLE_FAILED_DESC", nil, nil, @"The map failed to load because the style is corrupted.", @"User-friendly error description");
            break;
        case mbgl::MapLoadError::StyleLoadError:
            code = MLNErrorCodeLoadStyleFailed;
            description = NSLocalizedStringWithDefaultValue(@"LOAD_STYLE_FAILED_DESC", nil, nil, @"The map failed to load because the style can’t be loaded.", @"User-friendly error description");
            break;
        case mbgl::MapLoadError::NotFoundError:
            code = MLNErrorCodeNotFound;
            description = NSLocalizedStringWithDefaultValue(@"STYLE_NOT_FOUND_DESC", nil, nil, @"The map failed to load because the style can’t be found or is incompatible.", @"User-friendly error description");
            break;
        default:
            code = MLNErrorCodeUnknown;
            description = NSLocalizedStringWithDefaultValue(@"LOAD_MAP_FAILED_DESC", nil, nil, @"The map failed to load because an unknown error occurred.", @"User-friendly error description");
    }
    NSDictionary *userInfo = @{
        NSLocalizedDescriptionKey: description,
        NSLocalizedFailureReasonErrorKey: @(what.c_str()),
    };
    NSError *error = [NSError errorWithDomain:MLNErrorDomain code:code userInfo:userInfo];

    [mapView mapViewDidFailLoadingMapWithError:error];
}

void MLNMapViewImpl::onWillStartRenderingFrame() {
    [mapView mapViewWillStartRenderingFrame];
}

void MLNMapViewImpl::onDidFinishRenderingFrame(mbgl::MapObserver::RenderFrameStatus status) {
    bool fullyRendered = status.mode == mbgl::MapObserver::RenderMode::Full;
    [mapView mapViewDidFinishRenderingFrameFullyRendered:fullyRendered frameTime:status.frameTime];
}

void MLNMapViewImpl::onWillStartRenderingMap() {
    [mapView mapViewWillStartRenderingMap];
}

void MLNMapViewImpl::onDidFinishRenderingMap(mbgl::MapObserver::RenderMode mode) {
    bool fullyRendered = mode == mbgl::MapObserver::RenderMode::Full;
    [mapView mapViewDidFinishRenderingMapFullyRendered:fullyRendered];
}

void MLNMapViewImpl::onDidFinishLoadingStyle() {
    [mapView mapViewDidFinishLoadingStyle];
}

void MLNMapViewImpl::onSourceChanged(mbgl::style::Source& source) {
    NSString *identifier = @(source.getID().c_str());
    MLNSource * nativeSource = [mapView.style sourceWithIdentifier:identifier];
    [mapView sourceDidChange:nativeSource];
}

void MLNMapViewImpl::onDidBecomeIdle() {
    [mapView mapViewDidBecomeIdle];
}

void MLNMapViewImpl::onStyleImageMissing(const std::string& imageIdentifier) {
    NSString *imageName = [NSString stringWithUTF8String:imageIdentifier.c_str()];
    [mapView didFailToLoadImage:imageName];
}

bool MLNMapViewImpl::onCanRemoveUnusedStyleImage(const std::string &imageIdentifier) {
    NSString *imageName = [NSString stringWithUTF8String:imageIdentifier.c_str()];
    return [mapView shouldRemoveStyleImage:imageName];
}
