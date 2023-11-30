import os
import time
from pathlib import Path
from subprocess import Popen, TimeoutExpired

STREAM_URL = "https://www.youtube.com/watch?v=BahXDYWQAKk"

NUM_VIDEOS = 1000
DURATION = 2
# secs
DELAY = 60 * 5

RESOLUTION = "240p"

OUTPUT_FOLDER = "open_data-videos"

def main():
    output_folder = Path(OUTPUT_FOLDER)
    output_folder.mkdir(exist_ok=True)

    counter = 0
    for counter in range(NUM_VIDEOS):
        print(f"counter: {counter}/{NUM_VIDEOS}")
        id = int(time.time())
        path = os.path.join(output_folder, f"video-{id}.mp4")
        process = Popen(["streamlink", STREAM_URL, RESOLUTION, "-o", str(path), "--hls-duration", f"00:00:{DURATION}"])

        try:
            outs, errs = process.communicate()
        except TimeoutExpired as te:
            process.kill()

        process.wait()
        time.sleep(DELAY)


if __name__ == "__main__":
    main()
