% ECE471/571 project 1
% How to illustrate data points
%

% clear the figure
clf;

% load the testing set
load synth.tr;
Te = synth;

% extract the samples belonging to different classes
I = find(Te(:,3) == 0);  % find the row indices for the 1st class, labeled as 0
Te0 = Te(I,1:2);
save Te0;                % so that you can use it directly next time 

I = find(Te(:,3) == 1);  % find the row indices for the 2nd class, labeled as 1
Te1 = Te(I,1:2);
save Te1;


% case I MPP decision boundary
func1= @(x,y) 2.82480516 .* x + 3.392661.*y - 1.50559662;

% case II MPP decision boundary
func2= @(x,y) 1.635772362.*x + 11.02608677.*y - 5.4470979;

% case III MPP decision boundary
func3= @(x,y) 1.4676866.*x^2 + 4.565702.*x.*y -7.122196.*y^2 - 4.023077.*x - 14.8893.*y + 6.349892; 

% Likelihood ratio decision boundary
func4= @(x,y) 1.4676866.*x^2 + 4.565702.*x.*y -7.122196.*y^2 - 4.023077.*x - 14.8893.*y + 6.349892; 
  
% P0=0.5;
% P1=0.5;
% mu0=[-0.22147;0.325755];
% mu1=[0.0759543;0.682969];
% %S0=[0.27681 0.0112287;0.0112287 0.0361191];
% %S1=[0.159748 -0.015575;-0.015575 0.0299584];
% S0=[0.218279 -0.0043463;-0.0043463 0.03303875];
% S1=[0.218279 -0.0043463;-0.0043463 0.03303875];
% syms x y equation equation0 equation1; 
% equation0=-0.5*[x;y]'*inv(S0)*[x;y]+mu0'*inv(S0)'*[x;y]-0.5*mu0'*inv(S0)'*mu0-0.5*log(det(S0))+log(P0); 
% equation1=-0.5*[x;y]'*inv(S1)*[x;y]+mu1'*inv(S1)'*[x;y]-0.5*mu1'*inv(S1)'*mu1-0.5*log(det(S1))+log(P1); 
% equation=equation0-equation1;
    
% plot the samples
plot(Te0(:,1),Te0(:,2),'r*'); % use "red" for class 0
hold on;           % so that the future plots can be superimposed on the previous ones
plot(Te1(:,1),Te1(:,2),'go'); % use "green" for class 1

plot decision boundary
hold on;
h=ezplot(func4,[-1.2,1.3]); 
set(h,'Color','b');

title ('Test Samples Plot And Two Modal Gaussian');
xlabel ('x1');
ylabel ('x2');
legend('class 0','class 1','boundary','Location','southeast');








