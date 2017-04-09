% set parameters
nf=3;
c=4;
%load training set
load ('nfX23','-ascii');
Tr=nfX23;
TrX=Tr(:,1:nf);
TrL=Tr(:,nf+1);
save Trt;

%load testing set
load ('nfX1','-ascii');
Te=nfX1;
TeX=Te(:,1:nf);
TeL=Te(:,nf+1);
save Tet;

% fit /train decision tree
ctree= fitctree(TrX,TrL);
% predict using the trained tree
y=predict(ctree,TeX);

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
view(ctree,'mode','graph');

% write to a file
%dlmwrite('decisionCMndf12.txt',cm,' ');
%dlmwrite('decision_bks_nfX12.txt',[TeL y],' ');
