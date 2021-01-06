import numpy as np
from glob import glob
from os.path import basename
from sklearn.model_selection import train_test_split
from xgboost import XGBClassifier
from micromlgen import port

def load_features(folder):
    dataset = None
    classmap = {}
    class_idx = 0
    for dummy, filename in enumerate(glob('%s/*.csv' % folder)):
        print(filename + " laden...")      
        samples = np.loadtxt(filename, dtype=float, comments='#', delimiter=',')
        #print(samples)
        if (not samples.any()):
          continue;
        class_name = basename(filename)[:-4]
        classmap[class_idx] = class_name
        labels = np.ones((len(samples), 1)) * class_idx
        samples = np.hstack((samples, labels))
        dataset = samples if dataset is None else np.vstack((dataset, samples))
        class_idx = class_idx + 1
    return dataset, classmap

features, classmap = load_features('dataset/')
X, y = features[:, :-1], features[:, -1]

X_train, X_test, y_train, y_test = train_test_split(X, y, stratify=y, test_size=0.3, random_state=42)

clf = XGBClassifier(n_estimators=300, random_state=42)
clf.fit(X_train, y_train)
  
y_pred = clf.predict(X_test)
print(y_pred)

cppCode = port(clf, classmap=classmap)
print(cpp_code)
