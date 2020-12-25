function LUT = findLUT(algorithm,printer,config,var1,var2)
%FINDLUT 计算半调算法对应的LUT
%algorithm为半调算法，printer为打印机模型,config为打印机模型配置，其余为算法的参数

fun=func2str(algorithm);
filename=['calcLUT/',num2str(config.factor),'fac_',fun,num2str(var1),num2str(var2),'LUT.mat']; 
if(exist(filename,'file'))
    LUT1=load(filename,'-mat','LUT');
    LUT=LUT1.LUT
else
    disp('请耐心等待一分钟计算LUT，打印机的衰减参数第一次出现即未记录到查找表时才会执行该过程，以后便不用再执行')
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
    title([func2str(algorithm),'算法',num2str(var1),num2str(var2),'的灰度映射变化'])
    hold on
    plot(LUT);
    xlabel('原始灰度'),ylabel('映射灰度')
    save(filename,'LUT')
    disp('计算LUT完毕')
end
end

