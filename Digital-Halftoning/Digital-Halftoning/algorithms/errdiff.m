function [out] = errdiff(img,direction,flag,fc)
	% img -- 输入图像
	% flag -- 选用filter的参数
	% fc -- 用户自定义的filter
	% 如果输入参数等于4个，则使用用户自定义的filter
	% 如果输入参数等于3个，则按照flag来确定使用的filter
    % 1--Floyd_Steinberg filter; 0--Stucki filter
    % 如果输入参数等于2个，则按照direction来确定处理方向
    % 1--serpentine scan; 0--raster scan
	% 如果输入参数为1个，则默认使用Floyd_Steinberg filter，栅格搜索
	if nargin == 3
		if flag == 0
		    fc = [0 0 100*64 8 4; 2 4 8 4 2; 1 2 4 2 1]/64;    % Stucki filter
	    elseif flag == 1
		    fc = [0 100*16 7;3 5 1]/16;                        % Floyd_Steinberg filter
		end
	end
	if nargin < 3
		fc = [0 100*16 7;3 5 1]/16;
	end
    if nargin < 2
        direction = 0;
    end

	[r_in, c_in] = size(img);               % 输入图像的大小
    [r_fc, c_fc] = size(fc);                % filter的大小
    [r0, c0] = find(fc == 100);				% 处理值在filter中的位置
    fc(r0, c0) = 0;
    img_padding = zeros(r_in + r_fc - 1, c_in + c_fc - 1);    % 填充操作，以便操作所有的像素
    img_padding(r0:r0 + r_in - 1, c0:c0 + c_in - 1) = img;
    out = zeros(r_in, c_in);                                  % 预分配输出矩阵
    r0 = r0 - 1; c0 = c0 - 1;
    % 遍历的头、尾、步长
    head = 1; tail = c_in; steps = 1;

    for r = 1:r_in
    	for c = head:steps:tail
    		in_pixel = img_padding(r + r0, c + c0);           % 当前处理的像素
    		out_bool = in_pixel > 128;                        % 大于128置1，小于置0
    		out_pixel = out_bool * 255;
    		out(r, c) = out_pixel;
    		err = out_pixel - in_pixel;                       % 误差
    		fc_err = fc .* err;
    		img_padding(r:r+r_fc-1, c:c+c_fc-1) = img_padding(r:r+r_fc-1, c:c+c_fc-1) - fc_err;
    	end
        % 如果是按螺旋方式处理
        if direction == 1
            steps = -steps;
            temp = head;
            head = tail;
            tail = temp;
            % 将filter水平翻转
            fc=fc(:,c_fc:-1:1);
        end
    end
    out=uint8(out);
end