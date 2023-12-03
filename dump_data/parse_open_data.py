import os
import cv2
from pathlib import Path

INPUT_FOLDER = "open-videos"
OUTPUT_FOLDER = "open-images"

FPS = 1


def get_frame(path, sec, save_timestamp):
    video = cv2.VideoCapture(path)

    video.set(cv2.CAP_PROP_POS_MSEC, sec * 1000)

    has_frame, image = video.read()

    if has_frame:
        cv2.imwrite(os.path.join(OUTPUT_FOLDER, f"{save_timestamp}-{sec}.jpg"), image)

    return has_frame


def main():
    output_path = Path(OUTPUT_FOLDER)
    output_path.mkdir(exist_ok=True)

    video_names = os.listdir(INPUT_FOLDER)
    num_vidoes = len(video_names)

    for ix, video_name in enumerate(video_names):
        print(f"{ix}/{num_vidoes}")
        name, _ = video_name.split(".")
        _, timestamp = name.split("-")

        path = os.path.join(Path(INPUT_FOLDER), video_name)

        sec = 0
        success = True

        while success:
            success = get_frame(path, sec, timestamp)
            sec = sec + FPS
            sec = round(sec, 2)


if __name__ == "__main__":
    main()
