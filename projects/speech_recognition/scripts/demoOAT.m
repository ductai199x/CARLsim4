
clear all; close all;

% add Offline Analysis Toolbox to path
initOAT;

SR = SpikeReader('/home/sweet/1-workdir/carlsim4_ductai199x/projects/speech_recognition/results/spk_pooling.dat');
spk = SR.readSpikes(1000);
X = bsxfun(@minus,spk,mean(spk));
[coeffs, score, latent] = pca(X);

% X_train = [score(1201:1700,1:10); score(1801:2300,1:10)];
% X_test = [score(1701:1800,1:10); score(2301:2400,1:10)];

% X_train = [score(2400:2499,1:20); score(3600:3699,1:20)];
% X_test = [score(3200:3599,1:20); score(4400:end,1:20)];

X_train = [spk(1201:1700,1:4); spk(1801:2300,1:4)];
X_test = [spk(1701:1800,1:4); spk(2301:2400,1:4)];
% X_train = spk(size(spk,1)/4+1:size(spk,1)/2,:);
% X_train = [X_train; spk(size(spk,1)/4*3+1:end,:)];

Y_train = [ones(size(X_train, 1)/2,1); zeros(size(X_train, 1)/2,1)];
Y_test = [ones(size(X_test, 1)/2,1); zeros(size(X_test, 1)/2,1)];

% Y_train = [ones(size(X_train, 1)/2,1); ones(size(X_train, 1)/2,1)*-1];
% Y_test = [ones(size(X_test, 1),1)];

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
acc_train = 1 - sum(Y_train~=train_res)/length(Y_train)

train_res = predict(svm,X_test);
acc_test = 1 - sum(Y_test~=train_res)/length(Y_test)