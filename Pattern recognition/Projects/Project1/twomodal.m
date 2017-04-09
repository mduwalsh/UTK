% ECE471/571 project 1
% How to estimate two-modal Gaussian
% 
% Assume you've already run "plotsynth.m"

%
clear all;

% following is the estimate from class 0, the red samples
% if you've used the plotsynth.m to plot the samples from the training set
d = 2;    % two-dimensional
mu1 = [-0.75; 0.2];
mu2 = [0.3; 0.3];
S1 = [0.25 0; 0 0.3];
S2 = [0.1 0; 0 0.1];
A1 = 0.8;
A2 = 1 - A1;

nr = 1;     % row index
for i=-1.5:0.1:1
  nc = 1;   % column index
  for j=-0.2:0.1:1 
    x = [i;j];
    px1 = A1/((2*pi)^(d/2)*(det(S1))^(1/2)) * exp(-1/2*(x - mu1)'*inv(S1)*(x-mu1));
    px2 = A2/((2*pi)^(d/2)*(det(S2))^(1/2)) * exp(-1/2*(x - mu2)'*inv(S2)*(x-mu2));
    px(nr,nc) = px1 + px2;
    nc = nc + 1;
 end
 nr = nr + 1;
end

[m,n] = size(px);

xindex = repmat([-1.5:0.1:1]',1,n);
yindex = repmat([-0.2:0.1:1],m,1);

contour(xindex,yindex,px);