#import <AVFoundation/AVFoundation.h>
#import <Foundation/Foundation.h>
#import <ImageIO/ImageIO.h>

static BOOL writePNG(CGImageRef image, NSURL *url) {
  CGImageDestinationRef destination = CGImageDestinationCreateWithURL(
      (__bridge CFURLRef)url, CFSTR("public.png"), 1, NULL);
  if (destination == NULL) return NO;
  CGImageDestinationAddImage(destination, image, NULL);
  BOOL ok = CGImageDestinationFinalize(destination);
  CFRelease(destination);
  return ok;
}

int main(int argc, const char *argv[]) {
  @autoreleasepool {
    if (argc != 4) {
      fprintf(stderr, "usage: %s VIDEO OUTPUT_DIR FRAME_COUNT\n", argv[0]);
      return 2;
    }

    NSString *videoPath = [NSString stringWithUTF8String:argv[1]];
    NSString *outputPath = [NSString stringWithUTF8String:argv[2]];
    NSInteger frameCount = [[NSString stringWithUTF8String:argv[3]] integerValue];
    if (frameCount < 2 || frameCount > 24) {
      fprintf(stderr, "FRAME_COUNT must be between 2 and 24\n");
      return 2;
    }

    NSError *error = nil;
    [[NSFileManager defaultManager] createDirectoryAtPath:outputPath
                              withIntermediateDirectories:YES
                                               attributes:nil
                                                    error:&error];
    if (error != nil) {
      fprintf(stderr, "create output directory: %s\n", error.localizedDescription.UTF8String);
      return 1;
    }

    AVURLAsset *asset = [AVURLAsset URLAssetWithURL:[NSURL fileURLWithPath:videoPath]
                                            options:nil];
    Float64 durationSeconds = CMTimeGetSeconds(asset.duration);
    if (!isfinite(durationSeconds) || durationSeconds <= 0.0) {
      fprintf(stderr, "invalid video duration\n");
      return 1;
    }

    AVAssetImageGenerator *generator =
        [[AVAssetImageGenerator alloc] initWithAsset:asset];
    generator.appliesPreferredTrackTransform = YES;
    generator.requestedTimeToleranceBefore = kCMTimeZero;
    generator.requestedTimeToleranceAfter = kCMTimeZero;

    for (NSInteger index = 0; index < frameCount; ++index) {
      Float64 second = durationSeconds * (Float64)index / (Float64)frameCount;
      CMTime requested = CMTimeMakeWithSeconds(second, 600);
      CGImageRef image = [generator copyCGImageAtTime:requested
                                           actualTime:NULL
                                                error:&error];
      if (image == NULL) {
        fprintf(stderr, "extract frame %ld: %s\n", (long)index,
                error.localizedDescription.UTF8String);
        return 1;
      }

      NSString *name = [NSString stringWithFormat:@"circle_%02ld.png", (long)index];
      NSURL *outputURL = [NSURL fileURLWithPath:[outputPath stringByAppendingPathComponent:name]];
      BOOL wrote = writePNG(image, outputURL);
      CGImageRelease(image);
      if (!wrote) {
        fprintf(stderr, "write frame %ld failed\n", (long)index);
        return 1;
      }
    }

    NSInteger intervalMs = lround(durationSeconds * 1000.0 / (Float64)frameCount);
    printf("duration=%.3f frames=%ld interval_ms=%ld\n", durationSeconds,
           (long)frameCount, (long)intervalMs);
    return 0;
  }
}
