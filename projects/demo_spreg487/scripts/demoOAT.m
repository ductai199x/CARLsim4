
clear all; close all;

% add Offline Analysis Toolbox to path
initOAT;












folder_path = "/home/sweet/1-workdir/carlsim4_ductai199x/projects/speech_recognition/results/";
file_name = "spk_pooling.dat";

file_path = strcat(folder_path, file_name);

SR = SpikeReader(file_path);
spk = SR.readSpikes(1000);

X_train = [spk(r/2+1 : r/2+r/4*test_train_ratio, :); spk(r/2+r/4+1 : r/2+r/4*(1+test_train_ratio), :)];
X_test = [spk(r/2+r/4*test_train_ratio+1 : r/2+r/4, :); spk(r/2+r/4*(1+test_train_ratio)+1 : end, :)];

Y_train = [ones(size(X_train, 1)/2,1); zeros(size(X_train, 1)/2,1)];
Y_test = [ones(size(X_test, 1)/2,1); zeros(size(X_test, 1)/2,1)];

rng(0);

tol = 0.001;
C = 0.028507;
sigma = 11.716;


% svm = fitcsvm(X_train, Y_train,'KernelFunction','linear',...
%     'KernelScale',sigma,'BoxConstraint',C);

svm = fitcsvm(X_train, Y_train, 'OptimizeHyperparameters','auto',...
    'HyperparameterOptimizationOptions',...
    struct('MaxObjectiveEvaluations',100, 'UseParallel', true, 'Repartition', true));

% Computation of the error probability
train_res = predict(svm,X_train);
acc_train = 1 - sum(Y_train~=train_res)/length(Y_train)

train_res = predict(svm,X_test);
acc_test = 1 - sum(Y_test~=train_res)/length(Y_test)

% knn = fitcknn(X_train, Y_train, 'OptimizeHyperparameters','auto',...
%     'HyperparameterOptimizationOptions',...
%     struct('MaxObjectiveEvaluations',100, 'UseParallel', true, 'Repartition', true));
%     
% % Computation of the error probability
% train_res = predict(knn,X_train);
% acc_train = 1 - sum(Y_train~=train_res)/length(Y_train)
% 
% train_res = predict(knn,X_test);
% acc_test = 1 - sum(Y_test~=train_res)/length(Y_test)


