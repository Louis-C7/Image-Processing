function out=Bayers(x,num)
% x=imread('football.jpg');
y=im2bw(x);
% figure(20)
% imshow(y)
%x=rgb2gray(x);
% num=input('enter the dimension of the bayer matrix 2 or 4 or 8 or 16' );
x=imresize(x,[512 512]);
% figure(1)
% imshow(x)
B2=[1 2; 3 0];
M=num*num;
B4=[4*B2+1 4*B2+2; 4*B2+3 4*B2+0];
B8=[4*B4+1 4*B4+2; 4*B4+3 4*B4+0];
B16=[4*B8+1 4*B8+2; 4*B8+3 4*B8+0];
B=[0 1; 2 3];

%%
%T=round((255.*(B+0.5))/4);
%T=[32 223;159 96]
if num==2
    B=B2;
elseif num==4
    B=B4;
elseif num==8
    B=B8;
elseif num==16
    B=B16;
else
    disp('wrong value for bayer matrix');
end
T=round(255*(B+0.5)/M);
y=zeros(512,512);
c=0;
h=(512/num)-1;
for f=0:1:h
   for e=0:1:h
        c=c+1;
        for u=0:num-1
            for q=0:num-1
            r=(num*e)+1+u;
            d=(num*f)+1+q;
            if x(r,d)< T(u+1,q+1)
                y(r,d)=0;
            else
                y(r,d)=255;
            end
            end
        end
    end
end
out=y;
