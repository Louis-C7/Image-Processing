function varargout = halftone_dialog(varargin)
% Begin initialization code - DO NOT EDIT
gui_Singleton = 1;
gui_State = struct('gui_Name',       mfilename, ...
                   'gui_Singleton',  gui_Singleton, ...
                   'gui_OpeningFcn', @halftone_dialog_OpeningFcn, ...
                   'gui_OutputFcn',  @halftone_dialog_OutputFcn, ...
                   'gui_LayoutFcn',  [] , ...
                   'gui_Callback',   []);
if nargin && ischar(varargin{1})
    gui_State.gui_Callback = str2func(varargin{1});
end

if nargout
    [varargout{1:nargout}] = gui_mainfcn(gui_State, varargin{:});
else
    gui_mainfcn(gui_State, varargin{:});
end
% End initialization code - DO NOT EDIT


% --- Executes just before halftone_dialog is made visible.
function halftone_dialog_OpeningFcn(hObject, eventdata, handles, varargin)
% This function has no output args, see OutputFcn.
mainGuiInput = find(strcmp(varargin, 'mainapp'));
if(mainGuiInput)
    handles.main=varargin{mainGuiInput+1};
    handles.img=varargin{mainGuiInput+2};
    handles.imgname=regexprep(varargin{mainGuiInput+3},'datas/|.jpg|.png|.bmp|.tif','');
    handles.config = get_default_config();
    handles.printer=Printer(handles.config);
    addpath('algorithms')
    calcHalftone(handles,@errdiff,0,0);
    
else
    f = errordlg('Internal Error','Could not Related to the main');
    close halftone_dialog
end
% select default command line output for halftone_dialog
handles.output = hObject;
% Update handles structure
guidata(hObject, handles);

function figure1_DeleteFcn(hObject, eventdata, handles)
rmpath('algorithms')

% --- Outputs from this function are returned to the command line.
function varargout = halftone_dialog_OutputFcn(hObject, eventdata, handles) 
% Get default command line output from handles structure
varargout{1} = handles.output;

% --------------------------------------------------------------------
function select_Callback(hObject, eventdata, handles)
% --------------------------------------------------------------------
function Errdiff_Callback(hObject, eventdata, handles)
% --------------------------------------------------------------------
function Stucki_raster_Callback(hObject, eventdata, handles)
calcHalftone(handles,@errdiff,0,0);

% --------------------------------------------------------------------
function Stucki_serpentine_Callback(hObject, eventdata, handles)
calcHalftone(handles,@errdiff,1,0);

% --------------------------------------------------------------------
function Floyd_Raster_Callback(hObject, eventdata, handles)
calcHalftone(handles,@errdiff,0,1);

% --------------------------------------------------------------------
function Floyd_Serpentine_Callback(hObject, eventdata, handles)
calcHalftone(handles,@errdiff,1,1);

% --------------------------------------------------------------------
function dot_Callback(hObject, eventdata, handles)
% --------------------------------------------------------------------
function Messe_Callback(hObject, eventdata, handles)
calcHalftone(handles,@dotdiff,1,0);

% --------------------------------------------------------------------
function Knuth_Callback(hObject, eventdata, handles)
calcHalftone(handles,@dotdiff,2,0);

% --------------------------------------------------------------------
function other_Callback(hObject, eventdata, handles)
calcHalftone(handles,@dotdiff,3,0);

% --------------------------------------------------------------------
function Bayers_Callback(hObject, eventdata, handles)

% --------------------------------------------------------------------
function Dimension2_Callback(hObject, eventdata, handles)
calcHalftone(handles,@Bayers,2,0);

% --------------------------------------------------------------------
function Dimension4_Callback(hObject, eventdata, handles)
calcHalftone(handles,@Bayers,4,0);

% --------------------------------------------------------------------
function Dimension8_Callback(hObject, eventdata, handles)
calcHalftone(handles,@Bayers,8,0);

% --------------------------------------------------------------------
function Dimension16_Callback(hObject, eventdata, handles)
calcHalftone(handles,@Bayers,16,0);

% --------------------------------------------------------------------
function edbs_Callback(hObject, eventdata, handles)
calcHalftone(handles,@EDBS,0,0);

% --------------------------------------------------------------------
function calcHalftone(handles,algorithm,var1,var2)
%CALCHALFTONE 半调算法计算入口
%   handles为窗口的标识，algorithm为算法，其他为算法的参数
%请将多余的参数设为0
    handles.printer.factor=0.043;
    LUT=findLUT(algorithm,handles.printer,handles.config,var1,var2);
    [im,im_original]=input_processor(handles.img,handles.config,LUT,1);
    im_half=algorithm(im,var1,var2);
    [ssimval,~]=ssim(im_half,im_original);
    handles.text6.String=['SSIM Value Between Halftone Picture and Original Picture Of ',func2str(algorithm)...
,' Algorithm:',num2str(ssimval)];
    im_result= handles.printer.print(im_half);
    axes(handles.axes1)
    imshow(im_result),title('Printer Result With Gray Mapping')
    [ssimval,ssimmap] = ssim(im_result,im_original);
    axes(handles.axes2)
    imshow(ssimmap),title(['SSIM Picture With Gray Mapping(Value:',num2str(ssimval),')'])
    
    [im,im_original]=input_processor(handles.img,handles.config,LUT,0);
    im_half=algorithm(im,var1,var2);
    im_result= handles.printer.print(im_half);
    axes(handles.axes3)
    imshow(im_result),title('Printer Result Without Gray Mapping')
    [ssimval,ssimmap] = ssim(im_result,im_original);
    axes(handles.axes4)
    imshow(ssimmap),title(['SSIM Picture Without Gray Mapping(Value:',num2str(ssimval),')'])



