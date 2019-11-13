
clear all; close all;

% add Offline Analysis Toolbox to path
initOAT;

folder_path = "/home/sweet/1-workdir/carlsim4_ductai199x/projects/speech_recognition/results/";
file_name = "spk_pooling.dat";

file_path = strcat(folder_path, file_name);

SR = SpikeReader(file_path);
spk = SR.readSpikes(1000);
[r, c] = size(spk);
test_train_ratio = 0.8;
[coeffs, score, latent] = pca(spk);

% X_train = [score(1201:1700,1:10); score(1801:2300,1:10)];
% X_test = [score(1701:1800,1:10); score(2301:2400,1:10)];

% X_train = [score(2400:2499,1:20); score(3600:3699,1:20)];
% X_test = [score(3200:3599,1:20); score(4400:end,1:20)];

X_train = [spk(r/2+1 : r/2+r/4*test_train_ratio, :); spk(r/2+r/4+1 : r/2+r/4*(1+test_train_ratio), :)];
X_test = [spk(r/2+r/4*test_train_ratio+1 : r/2+r/4, :); spk(r/2+r/4*(1+test_train_ratio)+1 : end, :)];
% X_train = [spk(1201:1700,:); spk(1801:2300,:)];
% X_test = [spk(1701:1800,:); spk(2301:2400,:)];
% X_train = [score(1201:1700,1:5); score(1801:2300,1:5)];
% X_test = [score(1701:1800,1:5); score(2301:2400,1:5)];

Y_train = [ones(size(X_train, 1)/2,1); zeros(size(X_train, 1)/2,1)];
Y_test = [ones(size(X_test, 1)/2,1); zeros(size(X_test, 1)/2,1)];

% Y_train = [ones(size(X_train, 1)/2,1); ones(size(X_train, 1)/2,1)*-1];
% Y_test = [ones(size(X_test, 1),1)];

rng(0);

tol = 0.001;
C = 39.275;
sigma = 177.63;


% svm = fitcsvm(X_train, Y_train,'KernelFunction','linear',...
%     'KernelScale',sigma,'BoxConstraint',C);

% svm = fitcsvm(X_train, Y_train, 'OptimizeHyperparameters','auto',...
%     'HyperparameterOptimizationOptions',...
%     struct('MaxObjectiveEvaluations',50, 'UseParallel', true, 'Repartition', true));
% 
% %Computation of the error probability
% train_res = predict(svm,X_train);
% acc_train = 1 - sum(Y_train~=train_res)/length(Y_train)
% 
% train_res = predict(svm,X_test);
% acc_test = 1 - sum(Y_test~=train_res)/length(Y_test)

knn = fitcknn(X_train, Y_train, 'OptimizeHyperparameters','auto',...
    'HyperparameterOptimizationOptions',...
    struct('MaxObjectiveEvaluations',50, 'UseParallel', true, 'Repartition', true));
    
%Computation of the error probability
train_res = predict(knn,X_train);
acc_train = 1 - sum(Y_train~=train_res)/length(Y_train)

train_res = predict(knn,X_test);
acc_test = 1 - sum(Y_test~=train_res)/length(Y_test)


