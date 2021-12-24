import numpy as np
import json

# loads recorded data from a demo 
def load_demo_data(fname):
    print("Loading JSON demo data from " + fname)

    # open JSON file
    f = open(fname)
    data = json.load(f)

    return data

# formats demo data loaded as a json file from load_demo_data
def format_demo_data(data):
    print("Formatting raw JSON data for py-motmetrics IoU pairwise distances")

    result = []
    frames = data['frames']
    num_frames = len(frames)
    
    for i in range(0, num_frames):
        c_frame = []
        detections = frames[i]['detections']
        num_detections = len(detections)
        for j in range(0, num_detections):
            c_detection = detections[j]
            c_frame.append([c_detection['x'], c_detection['y'], c_detection['w'], c_detection['h']])
        result.append(np.array(c_frame))
    return np.array(result, dtype=object)

