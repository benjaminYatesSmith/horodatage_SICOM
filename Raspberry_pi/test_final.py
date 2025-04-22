#!/usr/bin/env python3
import time
import os
from datetime import datetime, timedelta
import numpy as np
from picamera2 import Picamera2
from PIL import Image, ImageDraw, ImageFont

def capture_burst_images(burst_count=5):
    photo_dir = "/home/canvbar/Images/Test"
    if not os.path.exists(photo_dir):
        os.makedirs(photo_dir)
    
    picam2 = Picamera2()
    
    video_config = picam2.create_still_configuration()
    video_config["main"]["size"] = (420, 560)
    picam2.configure(video_config)
    
    picam2.start()
    time.sleep(2) 
    
    # Listes pour stocker les images capturées en mémoire et leur timestamp
    frames = []
    timestamps = []
    
    # Phase d'acquisition rapide en rafale
    for i in range(burst_count):
        now = datetime.now()
        frame = picam2.capture_array()
        
        frames.append(frame)
        timestamps.append(now)
    
    picam2.stop()
    picam2.close()
    delta = timedelta(milliseconds = 635)
    
    for idx, (frame, timestamp) in enumerate(zip(frames, timestamps)):
        frame = np.rot90(frame, k=1)
        timestamp = timestamp + delta
        timestamp = timestamp.strftime("%Y-%m-%d_%H-%M-%S.%f")[:-3]
        timestamp = timestamp + "+-18ms"
        
        img = Image.fromarray(frame)
        draw = ImageDraw.Draw(img)
        
        try:
            font = ImageFont.truetype("LiberationSans-Regular.ttf", 32)
        except IOError:
            font = ImageFont.load_default()
        
        subtitle = timestamp  
        
        text_width, text_height = draw.textsize(subtitle, font=font)
        x = (img.width - text_width) / 2
        y = img.height - text_height - 10  # 10 pixels de marge en bas
        
        rectangle_margin = 10
        rect_coords = (x - rectangle_margin, y - rectangle_margin,
                       x + text_width + rectangle_margin, y + text_height + rectangle_margin)
        
        if img.mode != "RGBA":
            img = img.convert("RGBA")
        overlay = Image.new("RGBA", img.size, (0, 0, 0, 0))
        overlay_draw = ImageDraw.Draw(overlay)
        overlay_draw.rectangle(rect_coords, fill=(0, 0, 0, 150))
        img = Image.alpha_composite(img, overlay)
        
        draw = ImageDraw.Draw(img)
        draw.text((x, y), subtitle, font=font, fill="white")
        
        filename = os.path.join(photo_dir, f"photo_{timestamp}_{idx}.jpg")
        img = img.convert("RGB")  # Conversion en RGB pour sauvegarder en JPEG
        img.save(filename)
        print(f"Photo {idx+1} enregistrée sous : {filename}")

if __name__ == '__main__':
    # Capture en rafale de 5 images (modifiable avec burst_count)
    capture_burst_images(burst_count=20)





