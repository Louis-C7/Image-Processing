
function lut= lut_inverse(L)
% ��L�л�ȡLUT
% table
    LUT=uint8(L);
    n=size(LUT,1);
    % ȷ����������
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
                    M(i+1)=j-2;%�ҵ����Ҷ���ӽ���L��Ӧ�ĻҶ�
                else
                    M(i+1)=j-1;%�ҵ����Ҷ���ӽ���L��Ӧ�ĻҶ�
                end
                break;
            end
        end
    end
    % �ٴ�ȷ����������
    for i=2:n
        if(M(i-1)>M(i))
            M(i)=M(i-1);
        end
    end
    M(1)=0;
    M(n)=255;
    lut=M;
end

