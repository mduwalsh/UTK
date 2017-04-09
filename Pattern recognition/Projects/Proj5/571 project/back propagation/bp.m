% set parameters
nf=6;
c=4;
%load training set
load ndf23;
Tr=ndf23;
TrX=Tr(:,1:nf);
TrL=Tr(:,nf+1);

% load testing set
load ndf1;
Te=ndf1;
TeX=Te(:,1:nf);
TeL=Te(:,nf+1);

% prepare data for bp
% transpose inputs
iTr=transpose(TrX);
iTe=transpose(TeX);

% create target vectors
tTr=zeros(size(TrL,1),c);
tTe=zeros(size(TeL,1),c);
for i=1:size(TrL,1)
    tTr(i,TrL(i)+1)=1;
end
for i=1:size(TeL,1)
    tTe(i,TeL(i)+1)=1;
end
tTr=transpose(tTr);
tTe=transpose(tTe);

% create NN
net=patternnet(10);
% train the network
net=train(net,iTr,tTr);

% test using the network
oTe=net(iTe);

% get back class labels from output target vectors
y=vec2ind(oTe);
y=transpose(y);
y=y-1; % since class starts from 1 target output vectors

%performance calculation
k=0;
for i=1:size(TeL,1)
    if(y(i,1)~=TeL(i,1))
        k=k+1;
    end
end
disp('peformance = ');
disp(k/size(TeL,1));

% calculate confusion matrix
out=zeros(size(y,1),2);
out(:,1)=TeL;
out(:,2)=y;
cm=zeros(c+1,c+1);
cm(1,1)=0;
for i=1:c
	cm(1,i+1)=i;
    cm(i+1,1)=i;
end
for r=1:c
    for i=1:c		
        w=0;
        for j=1:size(TeL,1)			
            if ((out(j,2)==r-1)&&(out(j,1)==i-1))
                w=w+1;
            end
        end
        cm(i+1,r+1)=w;
    end
end
disp(cm);

% write to a file
dlmwrite('bpCMndf23.txt',cm,' ');


