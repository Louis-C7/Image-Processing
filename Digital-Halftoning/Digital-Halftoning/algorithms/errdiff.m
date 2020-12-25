function [out] = errdiff(img,direction,flag,fc)
	% img -- ����ͼ��
	% flag -- ѡ��filter�Ĳ���
	% fc -- �û��Զ����filter
	% ��������������4������ʹ���û��Զ����filter
	% ��������������3��������flag��ȷ��ʹ�õ�filter
    % 1--Floyd_Steinberg filter; 0--Stucki filter
    % ��������������2��������direction��ȷ��������
    % 1--serpentine scan; 0--raster scan
	% ����������Ϊ1������Ĭ��ʹ��Floyd_Steinberg filter��դ������
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

	[r_in, c_in] = size(img);               % ����ͼ��Ĵ�С
    [r_fc, c_fc] = size(fc);                % filter�Ĵ�С
    [r0, c0] = find(fc == 100);				% ����ֵ��filter�е�λ��
    fc(r0, c0) = 0;
    img_padding = zeros(r_in + r_fc - 1, c_in + c_fc - 1);    % ���������Ա�������е�����
    img_padding(r0:r0 + r_in - 1, c0:c0 + c_in - 1) = img;
    out = zeros(r_in, c_in);                                  % Ԥ�����������
    r0 = r0 - 1; c0 = c0 - 1;
    % ������ͷ��β������
    head = 1; tail = c_in; steps = 1;

    for r = 1:r_in
    	for c = head:steps:tail
    		in_pixel = img_padding(r + r0, c + c0);           % ��ǰ���������
    		out_bool = in_pixel > 128;                        % ����128��1��С����0
    		out_pixel = out_bool * 255;
    		out(r, c) = out_pixel;
    		err = out_pixel - in_pixel;                       % ���
    		fc_err = fc .* err;
    		img_padding(r:r+r_fc-1, c:c+c_fc-1) = img_padding(r:r+r_fc-1, c:c+c_fc-1) - fc_err;
    	end
        % ����ǰ�������ʽ����
        if direction == 1
            steps = -steps;
            temp = head;
            head = tail;
            tail = temp;
            % ��filterˮƽ��ת
            fc=fc(:,c_fc:-1:1);
        end
    end
    out=uint8(out);
end