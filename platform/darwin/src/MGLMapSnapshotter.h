#import <Foundation/Foundation.h>
#import "MGLTypes.h"
#import "MGLGeometry.h"
#import "MGLMapCamera.h"

NS_ASSUME_NONNULL_BEGIN

MGL_EXPORT
@interface MGLMapSnapshotOptions : NSObject

- (instancetype )initWithStyleURL:(NSURL* )styleURL size:(CGSize) size;

#pragma mark - Configuring the map

@property (nonatomic, readonly) NSURL* styleURL;

@property (nonatomic) double zoom;

@property (nonatomic) MGLMapCamera* mapCamera;

@property (nonatomic) MGLCoordinateBounds region;

#pragma mark - Configuring the image

@property (nonatomic, readonly) CGSize size;

@property (nonatomic) CGFloat scale;

@end

/**
 A block to processes the result or error of a snapshot request.
 
 The result will be either an `MGLImage*` or a `NSError*`
 
 @param snapshot The image that was generated or `nil` if an error occurred.
 @param error The eror that occured or `nil` when succesful.
 */
typedef void (^MGLMapSnapshotCompletionHandler)(MGLImage* _Nullable snapshot, NSError* _Nullable error);

MGL_EXPORT
@interface MGLMapSnapshotter : NSObject

- (instancetype)initWithOptions:(MGLMapSnapshotOptions* )options;

/**
 Starts the snapshot creation and executes the specified block with the result.
 
 @param completionHandler The block to handle the result in.
 */
- (void)startWithCompletionHandler:(MGLMapSnapshotCompletionHandler)completionHandler;

/**
 Starts the snapshot creation and executes the specified block with the result on the specified queue.
 
 @param queue The queue to handle the result on.
 @param completionHandler The block to handle the result in.
 */
- (void)startWithQueue:(dispatch_queue_t)queue completionHandler: (MGLMapSnapshotCompletionHandler)completionHandler;

/**
 Cancels the snapshot creation request, if any.
 */
- (void)cancel;

/**
 Indicates wether as snapshot is currently being generated.
 */
@property (nonatomic, readonly, getter=isLoading) BOOL loading;

@end

NS_ASSUME_NONNULL_END
