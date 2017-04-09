% extract the samples belonging to different classes
nf=18;
c=4;
load dataset;
Tr = dataset;
I0 = find(Tr(:,nf+1) == 0);  % find the row indices for the 1st class, labeled as 0
Tr0 = Tr(I0,1:nf);
I1 = find(Tr(:,nf+1) == 1);  % find the row indices for the 2nd class, labeled as 1
Tr1 = Tr(I1,1:nf);
I2 = find(Tr(:,nf+1) == 2);  % find the row indices for the 3rd class, labeled as 2
Tr2 = Tr(I2,1:nf);
I3 = find(Tr(:,nf+1) == 3);  % find the row indices for the 4th class, labeled as 3
Tr3 = Tr(I3,1:nf);

C0=cov(Tr0);
M0=mean(Tr0);
n0=size(Tr0,1);
S0=(n0-1)*C0;
C1=cov(Tr1);
M1=mean(Tr1);
n1=size(Tr1,1);
S1=(n1-1)*C0;
C2=cov(Tr2);
M2=mean(Tr2);
n2=size(Tr2,1);
S2=(n2-1)*C0;
C3=cov(Tr3);
M3=mean(Tr3);
n3=size(Tr3,1);
S3=(n3-1)*C0;

M=(n0*M0+n1*M1+n2*M2+n3*M2)/(n0+n1+n2+n3);
Sw=S0+S1+S2+S3;

sb0=n0*transpose(M0-M)*(M0-M);
sb1=n1*transpose(M1-M)*(M1-M);
sb2=n2*transpose(M2-M)*(M2-M);
sb3=n3*transpose(M3-M)*(M3-M);
Sb=sb0+sb1+sb2+sb3;

%disp(Sw);
%disp(Sb);
[V,D]=eig(Sb,Sw);

W=V(:,1:c-1);

fX=Tr(:,1:nf)*W;
lX=Tr(:,nf+1);
size(lX)
size(fX)
fTr=[fX lX];
dlmwrite('nfX',fTr, ' ');
dlmwrite('nfX1', fTr(1:282,:),' ');
dlmwrite('nfX2',fTr(283:282*2,:), ' ');
dlmwrite('nfX3',fTr(282*2+1:282*3,:), ' ');
dlmwrite('nfX12',[fTr(1:282,:);fTr(283:282*2,:)], ' ');
dlmwrite('nfX13', [fTr(1:282,:);fTr(282*2+1:282*3,:)],' ');
dlmwrite('nfX23',[fTr(283:282*2,:);fTr(282*2+1:282*3,:)], ' ');





