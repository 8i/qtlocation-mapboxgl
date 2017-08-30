#import "MGLMapSnapshotter.h"

#import <mbgl/actor/actor.hpp>
#import <mbgl/actor/scheduler.hpp>
#import <mbgl/util/geo.hpp>
#import <mbgl/map/map_snapshotter.hpp>
#import <mbgl/map/camera.hpp>
#import <mbgl/storage/default_file_source.hpp>
#import <mbgl/util/default_thread_pool.hpp>
#import <mbgl/util/string.hpp>
#import <mbgl/util/shared_thread_pool.hpp>

#import "MGLOfflineStorage_Private.h"
#import "MGLGeometry_Private.h"
#import "UIImage+MGLAdditions.h"

@implementation MGLMapSnapshotOptions

- (instancetype _Nonnull)initWithStyleURL:(NSURL* _Nonnull) styleURL size:(CGSize) size;
{
    self = [super init];
    if (self) {
        _styleURL = styleURL;
        _size = size;
        _scale = [UIScreen mainScreen].scale;
    }
    return self;
}

@end

@implementation MGLMapSnapshotter {
    
    std::shared_ptr<mbgl::ThreadPool> mbglThreadPool;
    std::unique_ptr<mbgl::MapSnapshotter> mbglMapSnapshotter;
    std::unique_ptr<mbgl::Actor<mbgl::MapSnapshotter::Callback>> snapshotCallback;
    
    BOOL loading;
}

- (instancetype)initWithOptions:(MGLMapSnapshotOptions*)options;
{
    self = [super init];
    if (self) {
        loading = false;
        
        mbgl::DefaultFileSource *mbglFileSource = [MGLOfflineStorage sharedOfflineStorage].mbglFileSource;
        mbglThreadPool = mbgl::sharedThreadPool();
        
        std::string styleURL = std::string([options.styleURL.absoluteString UTF8String]);
        
        // Size; taking into account the minimum texture size for OpenGL ES
        mbgl::Size size = {
            static_cast<uint32_t>(MAX(options.size.width, 64)),
            static_cast<uint32_t>(MAX(options.size.height, 64))
        };
        
        float pixelRatio = MAX(options.scale, 1);
        
        // Camera options
        mbgl::CameraOptions cameraOptions;
        if (CLLocationCoordinate2DIsValid(options.mapCamera.centerCoordinate)) {
            cameraOptions.center = MGLLatLngFromLocationCoordinate2D(options.mapCamera.centerCoordinate);
        }
        cameraOptions.angle = MAX(0, options.mapCamera.heading) * mbgl::util::DEG2RAD;
        cameraOptions.zoom = MAX(0, options.zoom);
        cameraOptions.pitch = MAX(0, options.mapCamera.pitch);
        
        // Region
        mbgl::optional<mbgl::LatLngBounds> region;
        if (!MGLCoordinateBoundsIsEmpty(options.region)) {
            region = MGLLatLngBoundsFromCoordinateBounds(options.region);
        }
        
        // Create the snapshotter
        mbglMapSnapshotter = std::make_unique<mbgl::MapSnapshotter>(*mbglFileSource, *mbglThreadPool, styleURL, size, pixelRatio, cameraOptions, region);
    }
    return self;
}

- (void)startWithCompletionHandler: (MGLMapSnapshotCompletionHandler)completion;
{
    loading = true;
    snapshotCallback = std::make_unique<mbgl::Actor<mbgl::MapSnapshotter::Callback>>(*mbgl::Scheduler::GetCurrent(), [=](std::exception_ptr mbglError, mbgl::PremultipliedImage image) {
        loading = false;
        if (mbglError) {
            NSString *description = [NSString stringWithUTF8String:mbgl::util::toString(mbglError).c_str()];
            NSDictionary *userInfo = @{NSLocalizedDescriptionKey: description};
            NSError *error = [NSError errorWithDomain:MGLErrorDomain code:1 userInfo:userInfo];
            dispatch_async(dispatch_get_main_queue(), ^{
                completion(nil, error);
            });
        } else {
            MGLImage *mglImage = [[MGLImage alloc] initWithMGLPremultipliedImage:std::move(image)];
            dispatch_async(dispatch_get_main_queue(), ^{
                completion(mglImage, nil);
            });
        }
    });
    mbglMapSnapshotter->snapshot(snapshotCallback->self());
}

- (void)startWithQueue:(dispatch_queue_t)queue completionHandler: (MGLMapSnapshotCompletionHandler)completion;
{
    dispatch_async(queue, ^{
        [self startWithCompletionHandler:completion];
    });
}

- (void)cancel;
{
    mbglMapSnapshotter.reset();
}

-(BOOL)isLoading;
{
    return loading;
}

@end
