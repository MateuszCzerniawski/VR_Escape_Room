import os
import tensorflow
import pandas as pd
import joblib
import cv2
import numpy as np

class AiModel:
    def __init__(self, working_directory):
        self.pca = joblib.load(os.path.join(working_directory, 'ai_models', 'pca.joblib'))
        self.features = self.pca.n_components_
        #self.model = tensorflow.keras.models.load_model(os.path.join(working_directory, 'ai_models', 'model.keras'))
        self.model = joblib.load(os.path.join(working_directory, 'ai_models', 'conv_model.joblib'))

    def use_pca(self, images):
        images = [cv2.threshold(img, 0, 1, cv2.THRESH_BINARY)[1] for img in images]
        images = [image.flatten() for image in images]
        images = self.pca.transform(images)
        return pd.DataFrame([pd.Series(i) for i in images])

    def use_model(self, images):
        #images = self.use_pca(images)

        images = [cv2.resize(img, (20, 20)) for img in images]
        images = [cv2.threshold(img, 0, 1, cv2.THRESH_BINARY)[1] for img in images]
        images = [np.expand_dims(img, axis=-1) for img in images]
        images = np.array(images, dtype=np.float32)

        probabilities = self.model.predict(images)
        predictions = [int(np.argmax(i)) for i in probabilities]
        return predictions
