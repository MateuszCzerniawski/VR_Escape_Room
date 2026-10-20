from flask import Flask, request, jsonify
import os
import tensorflow
import sys
import joblib
import numpy as np
import cv2
import image_manipulation as iman
import ai_handler as ai

if sys.stdout is None:
    sys.stdout = open(os.devnull, 'w')
if sys.stderr is None:
    sys.stderr = open(os.devnull, 'w')

workingDir = os.path.dirname(os.path.abspath(__file__))
manipulator = iman.Manipulator(200, 150)
model = ai.AiModel(workingDir)
app = Flask(__name__)


@app.route('/status', methods=['GET'])
def status():
    return jsonify({"status": "Online"}), 200


@app.route('/background', methods=['POST'])
def set_background():
    try:
        data = request.get_json()
        width = data.get('width')
        height = data.get('height')
        background = data.get('image')
        background = iman.decode_image(background, int(width), int(height))
        manipulator.set_background(background)
        print(f'{width}x{height}')
        return jsonify({"message": "background set correctly"}), 200
    except Exception as e:
        print(str(e))
        return jsonify({"error": f"Background setting failed: {str(e)}"}), 500


@app.route('/classify', methods=['POST'])
def classify():
    try:
        if manipulator.background is None:
            raise Exception('no background to distinct from!')
        data = request.get_json()
        image = data.get('image')
        width = data.get('width')
        height = data.get('height')
        print(f'{width}x{height}')
        image = iman.decode_image(image, int(width), int(height))
        images, horizontals, verticals, widths, heights = manipulator.parse_img(image)
        predictions = model.use_model(images)
        response = {"message": str(predictions),
                    "horizontals": str(horizontals),
                    "verticals": str(verticals),
                    "widths": str(widths),
                    "heights": str(heights)}
        print(response)
        return jsonify(response), 200
    except Exception as e:
        print(str(e))
        return jsonify({"error": f"Classification failed: {str(e)}"}), 500


if __name__ == '__main__':
    print(workingDir)
    app.run(host='127.0.0.1', port=2137, debug=False, use_reloader=False)
