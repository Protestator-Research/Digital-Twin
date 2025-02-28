import numpy as np
from numpy import array
import csv
from sklearn.datasets import load_iris
from sklearn.ensemble import IsolationForest
import matplotlib.pyplot as plt
from keras.layers import LSTM
from keras.models import Sequential
from keras.layers import Dense
import pandas as pd
import seaborn as sns
sns.set(color_codes=True)

class SVM:
    def __init__(self, learning_rate=0.001, lambda_param=0.01, n_iters=1000):
        self.lr = learning_rate
        self.lambda_param = lambda_param
        self.n_iters = n_iters
        self.w = None
        self.b = None

    def fit(self, X, y):
        n_samples, n_features = X.shape
        y_ = np.where(y <= 0, -1, 1)

        self.w = np.zeros(n_features)
        self.b = 0

        for _ in range(self.n_iters):
            for idx, x_i in enumerate(X):
                condition = y_[idx] * (np.dot(x_i, self.w) - self.b) >= 1
                if condition:
                    self.w -= self.lr * (2 * self.lambda_param * self.w)
                else:
                    self.w -= self.lr * (2 * self.lambda_param * self.w - np.dot(x_i, y_[idx]))
                    self.b -= self.lr * y_[idx]

    def predict(self, X):
        approx = np.dot(X, self.w) - self.b
        return np.sign(approx)

def plot_decision_boundary(X, y, model):
    plt.scatter(X[:, 0], X[:, 1], c=y, cmap='bwr')
    ax = plt.gca()
    xlim = ax.get_xlim()
    ylim = ax.get_ylim()

    xx, yy = np.meshgrid(np.linspace(xlim[0], xlim[1], 50), np.linspace(ylim[0], ylim[1], 50))
    xy = np.vstack([xx.ravel(), yy.ravel()]).T
    Z = model.predict(xy).reshape(xx.shape)

    ax.contourf(xx, yy, Z, alpha=0.3, cmap='bwr')
    plt.show()

def zscore(s, window, thresh=3, return_all=True):
    roll = np.roll(s,window)
    avg = roll.mean()
    std = roll.std(ddof=0)
    z = np.divide(np.subtract(s,avg),std)
    m = np.where(np.logical_and(z>=(-thresh), z<=thresh))   
    
    if return_all:
        return z, avg, std, m
    return np.where(m, s, avg)


data = np.zeros((7056,2))
i=0
with open('MLTempDataset1.csv', 'r' ) as theFile:
    reader = csv.DictReader(theFile)
    for line in reader:
        #print(line)
        data[i,:] = [i+1,line['Hourly_Temp']]
        i=i+1


print(data)
plt.plot(data[:, 0], data[:, 1])
plt.xlabel('Room Temp.')
plt.ylabel('Number of Points')
plt.title('Room Temp')
plt.show()

z, avg, std, m = zscore(np.array(data[:,1]),5)
print(z)
print(avg)
print(std)
print(m)

#svm = SVM(learning_rate=0.001, lambda_param=0.01, n_iters=1000)
#svm.fit(X, y)
#plot_decision_boundary(X, y, svm)

fitValues = data[:,1]


clf = IsolationForest(n_estimators=10, warm_start=True)
clf.fit(data)  # fit 10 trees

#clf.set_params(n_estimators=20)  # add 10 more trees
#clf.fit(X)  # fit the added trees

n_steps = 3
# split a univariate sequence into samples
def split_sequence(sequence, n_steps):
	X, y = list(), list()
	for i in range(len(sequence)):
		# find the end of this pattern
		end_ix = i + n_steps
		# check if we are beyond the sequence
		if end_ix > len(sequence)-1:
			break
		# gather input and output parts of the pattern
		seq_x, seq_y = sequence[i:end_ix], sequence[end_ix]
		X.append(seq_x)
		y.append(seq_y)
	return array(X), array(y)

X, y = split_sequence(fitValues, n_steps)

# Encoder
encoder = Sequential()
encoder.add(LSTM(32, activation='relu', input_shape=(20,1)))

# Decoder
decoder = Sequential()
decoder.add(LSTM(32, activation='relu', input_shape=(32,1)))
decoder.add(Dense(1))

# Autoencoder
autoencoder = Sequential([encoder, decoder])
autoencoder.compile(loss='mse', optimizer='adam')
history = autoencoder.fit(X, X, epochs=200, validation_split=0.05).history
autoencoder.summary()

fig, ax = plt.subplots(figsize=(14, 6), dpi=80)
ax.plot(history['loss'], 'b', label='Train', linewidth=2)
ax.plot(history['val_loss'], 'r', label='Validation', linewidth=2)
ax.set_title('Model loss', fontsize=16)
ax.set_ylabel('Loss (mae)')
ax.set_xlabel('Epoch')
ax.legend(loc='upper right')
plt.show()

value = data[0:20, 1].reshape(1,20,1)
X_pred = autoencoder.predict(value)
X_pred = X_pred.reshape(X_pred.shape[0], X_pred.shape[1])
#X_pred = pd.DataFrame(X_pred, columns=['y'])



scored = pd.DataFrame()
Xtest = X.reshape(X.shape[0], X.shape[1])
scored['Loss_mae'] = np.mean(np.abs(X_pred-Xtest), axis = 1)
plt.figure(figsize=(16,9), dpi=80)
plt.title('Loss Distribution', fontsize=16)
sns.histplot(scored['Loss_mae'], kde= True);
plt.show()
scored['Threshold'] = 0.275
scored['Anomaly'] = scored['Loss_mae'] > scored['Threshold']
scored.head()

Xtrain = data[0:1, 1].reshape(1,1,1)
X_pred_train = autoencoder.predict(data[0:30, 1].reshape(1,30,1))
X_pred_train = X_pred_train.reshape(X_pred_train.shape[0], X_pred_train.shape[1])
X_pred_train = pd.DataFrame(X_pred_train)

scored_train = pd.DataFrame()
scored_train['Loss_mae'] = np.mean(np.abs(X_pred_train-Xtrain), axis = 1)
scored_train['Threshold'] = 0.275
scored_train['Anomaly'] = scored_train['Loss_mae'] > scored_train['Threshold']
scored = pd.concat([scored_train, scored])
# plot bearing failure time plot
scored.plot(logy=True,  figsize=(16,9), ylim=[1e-2,1e2], color=['blue','red'])




#plt.plot(data[:, 0], yhat)
#plt.xlabel('Room Temp.')
#plt.ylabel('Number of Points')
#plt.title('Filtered Room Temp')
#plt.show()