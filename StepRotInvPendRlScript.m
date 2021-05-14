mdl = 'StepRotInvPendRlModel';
open_system(mdl)

numObs = 5;
obsInfo = rlNumericSpec([numObs 1]);
obsInfo.Name = 'observations';

numAct = 1;
actInfo = rlNumericSpec([numAct 1],'LowerLimit',-4,'UpperLimit', 4);
actInfo.Name = 'motor_torque';

blk = [mdl,'/RL Agent'];
env = rlSimulinkEnv(mdl,blk,obsInfo,actInfo);
 
obsInfo = getObservationInfo(env);
numObservations = obsInfo.Dimension(1);
actInfo = getActionInfo(env);

Ts = 0.02;
Tf = 25;

rng(0)

statePath = [
    imageInputLayer([numObservations 1 1], 'Normalization', 'none', 'Name', 'observation')
    fullyConnectedLayer(128, 'Name', 'CriticStateFC1')
    reluLayer('Name', 'CriticRelu1')
    fullyConnectedLayer(200, 'Name', 'CriticStateFC2')];

actionPath = [
    imageInputLayer([1 1 1], 'Normalization', 'none', 'Name', 'action')
    fullyConnectedLayer(200, 'Name', 'CriticActionFC1', 'BiasLearnRateFactor', 0)];

commonPath = [
    additionLayer(2,'Name', 'add')
    reluLayer('Name','CriticCommonRelu')
    fullyConnectedLayer(1, 'Name', 'CriticOutput')];

criticNetwork = layerGraph(statePath);
criticNetwork = addLayers(criticNetwork, actionPath);
criticNetwork = addLayers(criticNetwork, commonPath);
    
criticNetwork = connectLayers(criticNetwork,'CriticStateFC2','add/in1');
criticNetwork = connectLayers(criticNetwork,'CriticActionFC1','add/in2');

% figure
% plot(criticNetwork)

criticOptions = rlRepresentationOptions('LearnRate',1e-03,'GradientThreshold',1);
critic = rlRepresentation(criticNetwork,criticOptions,'Observation',{'observation'},obsInfo,'Action',{'action'},actInfo);

actorNetwork = [
    imageInputLayer([numObservations 1 1], 'Normalization', 'none', 'Name', 'observation')
    fullyConnectedLayer(128, 'Name', 'ActorFC1')
    reluLayer('Name', 'ActorRelu1')
    fullyConnectedLayer(200, 'Name', 'ActorFC2')
    reluLayer('Name', 'ActorRelu2')
    fullyConnectedLayer(1, 'Name', 'ActorFC3')
    tanhLayer('Name', 'ActorTanh1')
    scalingLayer('Name','ActorScaling','Scale',max(actInfo.UpperLimit))];
actorOptions = rlRepresentationOptions('LearnRate',5e-04,'GradientThreshold',1);
actor = rlRepresentation(actorNetwork,actorOptions,'Observation',{'observation'},obsInfo,'Action',{'ActorScaling'},actInfo);

agentOptions = rlDDPGAgentOptions(...
    'SampleTime',Ts,...
    'TargetSmoothFactor',1e-3,...
    'ExperienceBufferLength',1e6,...
    'MiniBatchSize',128);
agentOptions.NoiseOptions.Variance = 0.4;
agentOptions.NoiseOptions.VarianceDecayRate = 1e-5;

agent = rlDDPGAgent(actor,critic,agentOptions);

maxepisodes = 2000;
maxsteps = ceil(Tf/Ts);
trainingOptions = rlTrainingOptions(...
    'MaxEpisodes',maxepisodes,...
    'MaxStepsPerEpisode',maxsteps,...
    'ScoreAveragingWindowLength',5,...
    'Verbose',false,...
    'Plots','training-progress',...
    'StopTrainingCriteria','AverageReward',...
    'StopTrainingValue',100,...
    'SaveAgentCriteria','EpisodeReward',...
    'SaveAgentValue',100);

doTraining = false;

if doTraining    
    % Train the agent.
    trainingStats = train(agent,env,trainingOptions);
    % SAVE AGENT
    reset(agent); % Clears the experience buffer
    curDir = pwd;
    saveDir = 'savedAgents';
    cd(saveDir)
    save(['trainedAgent_3D_' datestr(now,'mm_DD_YYYY_HHMM')],'agent');
    save(['trainingResults_3D_' datestr(now,'mm_DD_YYYY_HHMM')],'trainingStats');
    cd(curDir)
else
    % Load pretrained agent for the example.
    load('savedAgents\trainedAgent_3D_05_06_2021_1433.mat','agent')
end

simOptions = rlSimulationOptions('MaxSteps',500);
experience = sim(env,agent,simOptions);
