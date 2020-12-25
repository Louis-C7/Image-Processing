function [im_ready,im_original]= input_processor(im,config,LUT,flag)
% 实现分辨率转换和灰度映射
%im为待处理图像,config为打印机设置，LUT为灰度映射待查找表
%flag为1时表示使用灰度映射，为0时不使用
rows = config.dpi * config.print_size_inches(1);
cols = config.dpi * config.print_size_inches(2);
im_original = imresize(im,[rows,cols]);
if(flag==1)
    im_ready=intlut(im_original,uint8(LUT));
else
    im_ready=im_original;
end
end

