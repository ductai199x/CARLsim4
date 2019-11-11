
clear all; close all;

% add Offline Analysis Toolbox to path
initOAT;

folder_path = "/home/sweet/1-workdir/carlsim4_ductai199x/projects/speech_recognition/results/";
file_name = "spk_pooling_attempt1.dat";

file_path = strcat(folder_path, file_name);

SR = SpikeReader(file_path);
spk = SR.readSpikes(1000);
[coeffs, score, latent] = pca(spk);

% X_train = [score(1201:1700,1:10); score(1801:2300,1:10)];
% X_test = [score(1701:1800,1:10); score(2301:2400,1:10)];

% X_train = [score(2400:2499,1:20); score(3600:3699,1:20)];
% X_test = [score(3200:3599,1:20); score(4400:end,1:20)];

% X_train = [spk(1201:1700,:); spk(1801:2300,:)];
% X_test = [spk(1701:1800,:); spk(2301:2400,:)];
X_train = [score(1201:1700,1:3); score(1801:2300,1:3)];
X_test = [score(1701:1800,1:3); score(2301:2400,1:3)];

Y_train = [ones(size(X_train, 1)/2,1); zeros(size(X_train, 1)/2,1)];
Y_test = [ones(size(X_test, 1)/2,1); zeros(size(X_test, 1)/2,1)];

% Y_train = [ones(size(X_train, 1)/2,1); ones(size(X_train, 1)/2,1)*-1];
% Y_test = [ones(size(X_test, 1),1)];

tol = 0.001;
C = 0.0081004;
sigma = 1.5148;


svm = fitcsvm(X_train, Y_train,'KernelFunction','linear',...
    'KernelScale',sigma,'BoxConstraint',C);

% svm = fitcsvm(X_train, Y_train, 'OptimizeHyperparameters','auto');
    
%Computation of the error probability
train_res = predict(svm,X_train);
acc_train = 1 - sum(Y_train~=train_res)/length(Y_train)

train_res = predict(svm,X_test);
acc_test = 1 - sum(Y_test~=train_res)/length(Y_test)