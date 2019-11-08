
clear all; close all;

% add Offline Analysis Toolbox to path
initOAT;

SR = SpikeReader('/home/sweet/1-workdir/carlsim4_ductai199x/projects/speech_recognition/results/spk_pooling.dat');
spk = SR.readSpikes(1000);

X_train = spk(1:size(spk,1)/2,:);
X_test = spk(size(spk,1)/2+1:end,:);

Y_train = [ones(size(X_train, 1)/2,1); ones(size(X_train, 1)/2,1)*-1];
Y_test = [ones(size(X_test, 1)/2,1); ones(size(X_test, 1)/2,1)*-1];

tol = 0.01;
C = 1;
sigma = 0.1;
% C = 7.0301;
% sigma = 5.8171;


svm = fitcsvm(X_train, Y_train,'KernelFunction','rbf',...
    'KernelScale',sigma,'BoxConstraint',C,...
    'Solver','SMO','KKTTolerance',tol,...
    'IterationLimit',20000,'CacheSize',10000);

% svm = fitcsvm(X_train, Y_train, 'OptimizeHyperparameters','auto');
    
%Computation of the error probability
train_res = predict(svm,X_train);
pe_train = sum(Y_train~=train_res)/length(Y_train)

train_res = predict(svm,X_test);
pe_test = sum(Y_test~=train_res)/length(Y_test)