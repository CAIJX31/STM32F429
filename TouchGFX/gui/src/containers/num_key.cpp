#include <gui/containers/num_key.hpp>

uint32_t input_num =0;

num_key::num_key()
{
	okCallback = 0;
}

void num_key::initialize()
{
    num_keyBase::initialize();
}

void num_key::Num_ok_function()
{
	Unicode::snprintf(textArea1Buffer,TEXTAREA1_SIZE,"%d",input_num);
	
	textArea1.resizeHeightToCurrentText();
	textArea1.invalidate();

	if (okCallback && okCallback->isValid())
	{
		okCallback->execute(input_num);
	}
	input_num =0;
}

void num_key::Num_0_function()
{
	input_num *= 10;
	input_num +=0;

	Unicode::snprintf(textArea1Buffer,TEXTAREA1_SIZE,"%d",input_num);
	
	textArea1.resizeHeightToCurrentText();
	textArea1.invalidate();
}

void num_key::Num_1_function()
{

	
	input_num *= 10;
	input_num +=1;
	
	
	
	Unicode::snprintf(textArea1Buffer,TEXTAREA1_SIZE,"%d",input_num);
	
	textArea1.resizeHeightToCurrentText();
	textArea1.invalidate();
	
}

void num_key::Num_2_function()
{
	input_num *= 10;
	input_num +=2;
	
	
	
	Unicode::snprintf(textArea1Buffer,TEXTAREA1_SIZE,"%d",input_num);
	
	textArea1.resizeHeightToCurrentText();
	textArea1.invalidate();
}

void num_key::Num_3_function()
{
	input_num *= 10;
	input_num +=3;
	
	
	
	Unicode::snprintf(textArea1Buffer,TEXTAREA1_SIZE,"%d",input_num);
	
	textArea1.resizeHeightToCurrentText();
	textArea1.invalidate();
}

void num_key::Num_4_function()
{

		input_num *= 10;
	input_num +=4;
	
	
	
	Unicode::snprintf(textArea1Buffer,TEXTAREA1_SIZE,"%d",input_num);
	
	textArea1.resizeHeightToCurrentText();
	textArea1.invalidate();
}

void num_key::Num_5_function()
{
	input_num *= 10;
	input_num +=5;
	
	
	
	Unicode::snprintf(textArea1Buffer,TEXTAREA1_SIZE,"%d",input_num);
	
	textArea1.resizeHeightToCurrentText();
	textArea1.invalidate();
}

void num_key::Num_6_function()
{
	input_num *= 10;
	input_num +=6;
	
	
	
	Unicode::snprintf(textArea1Buffer,TEXTAREA1_SIZE,"%d",input_num);
	
	textArea1.resizeHeightToCurrentText();
	textArea1.invalidate();
}

void num_key::Num_7_function()
{
	input_num *= 10;
	input_num +=7;
	
	
	
	Unicode::snprintf(textArea1Buffer,TEXTAREA1_SIZE,"%d",input_num);
	
	textArea1.resizeHeightToCurrentText();
	textArea1.invalidate();
}
void num_key::Num_8_function()
{
	input_num *= 10;
	input_num +=8;
	
	
	
	Unicode::snprintf(textArea1Buffer,TEXTAREA1_SIZE,"%d",input_num);
	
	textArea1.resizeHeightToCurrentText();
	textArea1.invalidate();
}

void num_key::Num_9_function()
{
	input_num *= 10;
	input_num +=9;
	
	
	
	Unicode::snprintf(textArea1Buffer,TEXTAREA1_SIZE,"%d",input_num);
	
	textArea1.resizeHeightToCurrentText();
	textArea1.invalidate();
}

void num_key::Num_det_function()
{
	input_num /= 10;

	
	
	
	Unicode::snprintf(textArea1Buffer,TEXTAREA1_SIZE,"%d",input_num);
	
	textArea1.resizeHeightToCurrentText();
	textArea1.invalidate();
}
