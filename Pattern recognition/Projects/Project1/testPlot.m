% How to estimate two-modal Gaussian
% 
% Assume you've already run "plotsynth.m"

%
clear all;
% load the testing set
load synth.te;
Te = synth;

% extract the samples belonging to different classes
I0 = find(Te(:,3) == 0);  % find the row indices for the 1st class, labeled as 0
Te0 = Te(I0,1:2);
save Te0;                % so that you can use it directly next time 

I1 = find(Te(:,3) == 1);  % find the row indices for the 2nd class, labeled as 1
Te1 = Te(I1,1:2);
save Te1;
S=Te1;
i=1;
display(Te1(i,:));