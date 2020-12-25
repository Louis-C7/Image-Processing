function [im_ready,im_original]= input_processor(im,config,LUT,flag)
% ʵ�ֱַ���ת���ͻҶ�ӳ��
%imΪ������ͼ��,configΪ��ӡ�����ã�LUTΪ�Ҷ�ӳ������ұ�
%flagΪ1ʱ��ʾʹ�ûҶ�ӳ�䣬Ϊ0ʱ��ʹ��
rows = config.dpi * config.print_size_inches(1);
cols = config.dpi * config.print_size_inches(2);
im_original = imresize(im,[rows,cols]);
if(flag==1)
    im_ready=intlut(im_original,uint8(LUT));
else
    im_ready=im_original;
end
end

