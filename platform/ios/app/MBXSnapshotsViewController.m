#import "MBXSnapshotsViewController.h"

#import <Mapbox/Mapbox.h>

@interface MBXSnapshotsViewController ()

@property (weak, nonatomic) IBOutlet UIImageView *snapshotImageView;

@end

@implementation MBXSnapshotsViewController {
    MGLMapSnapshotter* snapshotter;
}

- (void)viewDidLoad {
    [super viewDidLoad];

    // Create snapshot options
    MGLMapSnapshotOptions* options = [[MGLMapSnapshotOptions alloc] initWithStyleURL:[NSURL URLWithString:@"mapbox://styles/mapbox/traffic-day-v2"] size:CGSizeMake(_snapshotImageView.frame.size.width, _snapshotImageView.frame.size.height)];
    options.mapCamera = [[MGLMapCamera alloc] init];
    options.mapCamera.pitch = 20;
    options.mapCamera.centerCoordinate = CLLocationCoordinate2DMake(52.0810, 5.1065);
    options.zoom = 10;
    
    // Create and start the snapshotter
    snapshotter = [[MGLMapSnapshotter alloc] initWithOptions:options];
    [snapshotter startWithCompletionHandler: ^(UIImage *image, NSError *error) {
        if (error) {
            NSLog(@"Could not load snapshot: %@", [error localizedDescription]);
        } else {
            _snapshotImageView.image = image;
        }
    }];
}


@end
