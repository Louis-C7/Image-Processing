classdef Printer
    %打印机模型.
    properties
        config
        factor
    end
    methods
        function obj = Printer(config)
            obj.config=config;
            obj.factor=config.factor;
        end
        function  im_result=print(obj, image)
            %Note that the image should be in uint8 format
            im_result=image;
            nearimage=image;
            kernel=[0 1 0
                    1 0 1
                    0 1 0];
            img = conv2(image,kernel,'same')./255;
            nearimage=4-img;
            nearimage=nearimage.*255.*obj.factor;
            for i=1:size(image,1)%点增益畸变
                for j=1:size(image,2)
                    if(image(i,j)==0)
                        im_result(i,j)=38;
                    else
                        im_result(i,j)=255-nearimage(i,j);
                    end
                end
            end
            im_result=uint8(im_result);
        end
    end
end