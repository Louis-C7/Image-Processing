function varargout = mainapp(varargin)
% Begin initialization code - DO NOT EDIT
gui_Singleton = 1;
gui_State = struct('gui_Name',       mfilename, ...
                   'gui_Singleton',  gui_Singleton, ...
                   'gui_OpeningFcn', @mainapp_OpeningFcn, ...
                   'gui_OutputFcn',  @mainapp_OutputFcn, ...
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

% --- Executes just before mainapp is made visible.
function mainapp_OpeningFcn(hObject, eventdata, handles, varargin)
% This function has no output args, see OutputFcn.
% Choose default command line output for mainapp
clc
handles.output = hObject;
filename='datas/human.jpg';
handles.img=rgb2gray(imread(filename));
handles.imgname=filename;
axes(handles.axes1)
imshow(handles.img)

% Update handles structure
guidata(hObject, handles);

% --- Outputs from this function are returned to the command line.
function varargout = mainapp_OutputFcn(hObject, eventdata, handles)
% Get default command line output from handles structure
varargout{1} = handles.output;


% --------------------------------------------------------------------
function uipushtool1_ClickedCallback(hObject, eventdata, handles)
file=uigetfile({'*.jpg;*.png;*.tif;*.bmp',...
    'Picture File (*.jpg;*.png;*.tif;*.bmp)'},'Select A Picture File');
filename=['datas/',file];
if(file~=0)
    handles.img=rgb2gray(imread(filename));
    handles.imgname=filename;
    axes(handles.axes1)
    imshow(handles.img)
    % Update handles structure
    guidata(hObject, handles);
end

% --------------------------------------------------------------------
function halftone_Callback(hObject, eventdata, handles)
%进入半调算法的窗口
halftone_dialog('mainapp', handles.figure1,handles.img,handles.imgname);
