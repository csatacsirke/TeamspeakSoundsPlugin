// Gui/AudioProcessorDialog.cpp : implementation file
//

#include "stdafx.h"
#include "Gui/AudioProcessorDialog.h"
#include "afxdialogex.h"

#include <Wave/AudioProcessor.h>


// AudioProcessorDialog dialog

IMPLEMENT_DYNAMIC(AudioProcessorDialog, CDialogEx)

AudioProcessorDialog::AudioProcessorDialog(AudioProcessor& audioProcessor, CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DIALOG1, pParent), audioProcessor(audioProcessor)
{
	
}

AudioProcessorDialog::~AudioProcessorDialog()
{
}

void AudioProcessorDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_ENABLE_CHECKBOX, enableCheckbox);
	DDX_Control(pDX, IDC_WINDOW_SIZE_MILLISECS_EDIT, windowSizeMillisecsEdit);



	if(pDX->m_bSaveAndValidate) {
		audioProcessor.enabled = enableCheckbox.GetCheck() != FALSE;
		//audioProcessor.windowLengthMicroSec = windowSizeMillisecsEdit.GetWindowText();
	} else {
		enableCheckbox.SetCheck((int)audioProcessor.enabled);
	}
}


//BOOL AudioProcessorDialog::OnInitDialog() {
//	
//
//	return TRUE;
//}

BEGIN_MESSAGE_MAP(AudioProcessorDialog, CDialogEx)
	ON_BN_CLICKED(IDC_ENABLE_CHECKBOX, &AudioProcessorDialog::OnBnClickedEnableCheckbox)
	ON_BN_CLICKED(IDC_BUTTON1, &AudioProcessorDialog::OnBnClickedButton1)
END_MESSAGE_MAP()


// AudioProcessorDialog message handlers


void AudioProcessorDialog::OnBnClickedEnableCheckbox() {
	//audioProcessor.enabled = enableCheckbox.GetCheck() != FALSE;
	UpdateData(TRUE);
	//BOOL enable = enableCheckbox.GetCheck();
	//if(enable) {
	//	audioProcessor.Enable();
	//} else {
	//	audioProcessor.Disable();
	//}
	//	

}


void AudioProcessorDialog::OnBnClickedButton1() {
	UpdateData(TRUE);
}
