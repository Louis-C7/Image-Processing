function out=Bayers(x,num,var2)
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
    fprint('wrong value for bayer matrix');
end
T=round(255*(B+0.5)/M);
y=x;
c=0;
[nrow,ncol]=size(x);
h_row=(nrow/num)-1;
h_col=(ncol/num)-1;
for f=0:1:h_col
   for e=0:1:h_row
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
