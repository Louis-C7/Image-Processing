
function lut= lut_inverse(L)
% 从L中获取LUT
% table
    LUT=uint8(L);
    n=size(LUT,1);
    % 确保单调递增
    for i=2:n
        if(LUT(i-1)>LUT(i))
            LUT(i)=LUT(i-1)+1;
        end
    end
    M=zeros(n,1);
    for i=0:n-1
        for j=1:n-1
            if(LUT(j)>i)
                if(j>1&(LUT(j)-i)>(i-LUT(j-1)))
                    M(i+1)=j-2;%找到跟灰度最接近的L对应的灰度
                else
                    M(i+1)=j-1;%找到跟灰度最接近的L对应的灰度
                end
                break;
            end
        end
    end
    % 再次确保单调递增
    for i=2:n
        if(M(i-1)>M(i))
            M(i)=M(i-1);
        end
    end
    M(1)=0;
    M(n)=255;
    lut=M;
end

