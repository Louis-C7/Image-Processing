function LUT = findLUT(algorithm,printer,config,var1,var2)
%FINDLUT �������㷨��Ӧ��LUT
%algorithmΪ����㷨��printerΪ��ӡ��ģ��,configΪ��ӡ��ģ�����ã�����Ϊ�㷨�Ĳ���

fun=func2str(algorithm);
filename=['calcLUT/',num2str(config.factor),'fac_',fun,num2str(var1),num2str(var2),'LUT.mat']; 
if(exist(filename,'file'))
    LUT1=load(filename,'-mat','LUT');
    LUT=LUT1.LUT
else
    disp('�����ĵȴ�һ���Ӽ���LUT����ӡ����˥��������һ�γ��ּ�δ��¼�����ұ�ʱ�Ż�ִ�иù��̣��Ժ�㲻����ִ��')
    table=zeros(256,2);
    for i=0:255
        colorblock=uint8(zeros(80,80));
        colorblock(:)=i;
        im_res=algorithm(colorblock,var1,var2);
        im_print=printer.print(im_res);
        table(i+1,1)=i;
        table(i+1,2)=round(mean(im_print(:)));
    end
    LUT=lut_inverse(table(:,2));
    figure
    title([func2str(algorithm),'�㷨',num2str(var1),num2str(var2),'�ĻҶ�ӳ��仯'])
    hold on
    plot(LUT);
    xlabel('ԭʼ�Ҷ�'),ylabel('ӳ��Ҷ�')
    save(filename,'LUT')
    disp('����LUT���')
end
end

