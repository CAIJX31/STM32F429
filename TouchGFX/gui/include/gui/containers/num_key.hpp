#ifndef NUM_KEY_HPP
#define NUM_KEY_HPP

#include <gui_generated/containers/num_keyBase.hpp>
#include <touchgfx/Callback.hpp>

class num_key : public num_keyBase
{
public:
    num_key();
    virtual ~num_key() {}

    virtual void initialize();
		virtual void Num_ok_function();
		virtual void Num_0_function();
		virtual void Num_1_function();
		virtual void Num_2_function();
		virtual void Num_3_function();
		virtual void Num_4_function();
	  virtual void Num_5_function();
		virtual void Num_6_function();
		virtual void Num_7_function();
		virtual void Num_8_function();
		virtual void Num_9_function();
		virtual void Num_det_function();		
    
    void setOkCallback(touchgfx::GenericCallback<uint32_t>& cb) { okCallback = &cb; }
protected:
    touchgfx::GenericCallback<uint32_t>* okCallback;
};

#endif // NUM_KEY_HPP
