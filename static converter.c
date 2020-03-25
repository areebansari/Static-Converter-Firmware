#include "D:\3kva Inverter (15-01-2014)\final work\software final version\Static Converter.h"

void main()
{
   int16 v_in,v_in1,v_in2,v_in3,v_max=715,v_min=410,v_zero=50,v_inp,v_inp1;
   int16 i_out,i_out1,i_out2,i_out3,i_max=570,i_ac_surge=810,i_out_old=0,
         i_out_old1=0;
   int16 v_ac=0,v_ac1=0,v_ac2=0,v_ac3=0,v_acf=0,v_ac4=0,v_ac5=0,v_ac6=0,
         v_ac_low=335,v_ac_high=480;
   int16 c_start=0,c_low=0,c_high=0,c_current=0,c_temp=0,c_ac_low=0,
         c_ac_high=0;
   int16 out_step;
   int16 t_1,t1_1,t2_1,t3_1,t_2,t1_2,t2_2,t3_2,t_3,t1_3,t2_3,t3_3,
         t_high=265;
   int   decision,count=0,ep1=0,k=0;
   int loop_number=0,a=0,b=0,c=0;

   setup_adc_ports(ALL_ANALOG);
   setup_adc(ADC_CLOCK_INTERNAL);
   setup_psp(PSP_DISABLED);
   setup_spi(FALSE);
   setup_timer_0(RTCC_INTERNAL|RTCC_DIV_1);
   setup_timer_1(T1_DISABLED);
   setup_timer_2(T2_DISABLED,0,1);
   // setup_comparator(NC_NC_NC_NC);
   // setup_vref(FALSE);

   // TODO: USER CODE!!  
   output_high(PIN_C5);
   output_high(PIN_C6);       
   output_high(PIN_C7);      
 
   set_adc_channel(0);
   delay_ms(1);
   v_in1=read_adc();
   delay_ms(1);
   v_in2=read_adc();
   delay_ms(1);
   v_in3=read_adc();
   v_in=((v_in1 + v_in2 + v_in3)/3);
  
   if(v_in < v_zero)
   {
      decision=0;
   }
   else
   {
      decision=1;
   } 
 
   While(TRUE)
   {
      v_inp1=v_inp;        //Digital Filtering
      v_inp=v_in;          //Digital Filtering
      set_adc_channel(0);  //DC Voltage
      delay_ms(1);
      v_in1=read_adc();  
      delay_ms(1);
      v_in2=read_adc();
      delay_ms(1);
      v_in3=read_adc();
      v_in=((v_in1 + v_in2 + v_in3+v_inp+v_inp1)/5);
  
      if(v_in > v_min)        //DC Capacitor charged
      {                        
         output_low(PIN_C1);   //Turn on Cahrging LED 
         if(ep1==0)
         {
            output_low(PIN_C3);//Disable SSR
         }
         else
         {
            output_high(PIN_C3);  //Enable SSR
         }
      }
      else
      {
         output_high(PIN_C1);
      }
  
      set_adc_channel(1);   //AC O/P Volt
      delay_ms(1);
      v_ac=read_adc();
      delay_ms(1);
   
      set_adc_channel(2);  //AC O/P Current
      delay_ms(1);
      i_out1=read_adc();
      delay_ms(1);
      i_out2=read_adc();
      delay_ms(1);
      i_out3=read_adc();
      i_out = ((i_out1 + i_out2 + i_out3)/3);
      i_out_old1=i_out_old;
      i_out_old=i_out;
      i_out = ((i_out_old+i_out_old1)/2);
  
      set_adc_channel(3);  // IGBT 1 temperature
      delay_ms(1);
      t1_1=read_adc();
      delay_ms(1);
      t2_1=read_adc();
      delay_ms(1);
      t3_1=read_adc();
      delay_ms(1);
      t_1= ((t1_1 + t2_1 + t3_1)/3);
  
      set_adc_channel(4);  // IGBT 2 temperature
      delay_ms(1);
      t1_2=read_adc();
      delay_ms(1);
      t2_2=read_adc();
      delay_ms(1);
      t3_2=read_adc();
      delay_ms(1);
      t_2= ((t1_2 + t2_2 + t3_2)/3);
   
      set_adc_channel(5);  // IGBT 3 temperature
      delay_ms(1);
      t1_3=read_adc();
      delay_ms(1);
      t2_3=read_adc();
      delay_ms(1);
      t3_3=read_adc();
      delay_ms(1);
      t_3= ((t1_3 + t2_3 + t3_3)/3);
  
      Switch(decision)
      {
         Case 0:                          //Ready To Start
         {
            output_d(0x00);         //Min: Voltage Selected
            ep1=1;                  //set Flag Enable
            output_bit(PIN_C0,ep1); //EEPROM O/P
            out_step=0;             //Reset all delay counters
            c_start=0;                
            c_low=0;
            c_high=0;
            c_current=0;
            c_temp=0;
            c_ac_low=0;
            c_ac_high=0;
          
            if(v_in > v_zero)    //Capacitor charging has Started
            {                    //So goto case 2
               decision=2;
            }
         }
         Break;
         
         Case 1:    //Block operation to protect IGBT from start up surge
         {
            ep1=0;
            output_bit(PIN_C0,ep1);  //EEPROM O/P Block
          
            if(v_in < v_zero)     //Capacitor discharging test 
            {                     //to send to case 0
               decision=0;
            }
         }
         Break;
   
         Case 2:    //Charging Time Monitor
         {              
            c_start++;   //Check for time alloted to charge the capacitors
           
            if(c_start > 200)    //Excess time result case 4
            {
               decision=4;
            }
          
            if(v_in > v_min)   //Charging voltage has reached the  
            {                  //set value with in time result case 3
               decision=3;
            }
         }
         Break;
   
         Case 3:                       // In operation
         {
            if(v_in < v_min)  //DC low voltage Trip
            {
               c_low++;         //Timer Counting
            }
            else
            {
               c_low=0;         //Reset/Stop Timer   
            }
          
            if(c_low > 100)  //Delayed DC low voltage trip
            {
               decision=4;
               output_low(PIN_B4);  //Low DC Indication 
            }
               
            if(v_in > v_max)  //DC high voltage Trip
            {
               c_high++;        //Timer Counting
            } 
            else
            {
               c_high=0;        //Reset/stop timer
            }
          
            if(c_high > 100)   //Delayed DC high voltage trip
            {
               decision=4;
               output_low(PIN_B3); // High DC indication
            }
 
            if(i_out > i_max)    //Over Load Trip
            {
               c_current++;      //Timer Counting
            }
            else
            {
               c_current=0;      //Reset/stop timing
            }
          
            if(c_current > 125) //Delayed overload Trip 
            {
               decision=4;
               output_low(PIN_B0);  //Over load indication
            }
        
            if(t_1<t_high || t_2<t_high || t_3<t_high) //High Temp: Trip
            {
               c_temp++;
            }
            else
            {
               c_temp=0;
            }
          
            if(c_temp > 150)    //delayed high Temp: trip    
            {
               decision=4;
               output_low(PIN_C7);  //high temp: indication
            }        
                    
            if(v_ac < v_ac_low)     //O/P AC low Volt
            {
               c_ac_low++;           //Timer Counting 
            }
            else
            {
               c_ac_low=0;           //Reset/Stop Timing 
            }
          
            if(c_ac_low > 100)    //Delayed O/P low voltage trip
            {
               decision=4;
               output_low(PIN_B2);
            }
               
            if(v_ac > v_ac_high)          //O/P AC High Volt
            {
               c_ac_high++;              //Timer Counting 
            }
            else
            {
               c_ac_high=0;              //Reset/Stop Timing
            }
          
            if(c_ac_high > 100)   //Delayed O/P high voltage trip
            {
               decision=4;
               output_low(PIN_B1);  //AC high volt indication
            }     
          
            if(!input(PIN_C2))   //Error Signal
            {
               decision=4;
               output_low(PIN_C6);  //IGBT Gate trip indication
            }
          
            if(i_out > i_ac_surge)  //Surge Trip
            {
               decision=4;
               output_low(PIN_C5);  //high surge indication
            }
         }
         Break;
   
         Case 4:                      //Trip State
         {
            ep1=0;               //Clear Flag to disable EEPROM
            output_bit(PIN_C0,ep1);  //disable EEPROM
            output_low(PIN_C3);  //disable SSR
         }
         Break;
      }
   
      if(i_out > i_ac_surge)    //quick current surge trip
      {
         decision=4;
      }
  
      if(!input(PIN_C2))       //IGBT gate drive trip
      {
         decision=4;
      }
  
      if(v_in < v_zero)        //If capacitor are discharge Case 0  
      {
         decision=0;
         output_high(PIN_B4);
         output_high(PIN_B3);
         output_high(PIN_B2);
         output_high(PIN_B1);
         output_high(PIN_B0);
         output_high(PIN_C7);
         output_high(PIN_C6);
         output_high(PIN_C5);
      }
   
      loop_number++;
      if (loop_number<100)
      {
         if(v_in > 396) {out_step=31; }   //Steps Selection from 0-31
         if(v_in > 402) {out_step=30; }   //with respect to input DC volt  
         if(v_in > 409) {out_step=29; }
         if(v_in > 416) {out_step=28; }
         if(v_in > 424) {out_step=27; }
         if(v_in > 431) {out_step=26; } 
         if(v_in > 439) {out_step=25; }
         if(v_in > 447) {out_step=24; }
         if(v_in > 455) {out_step=23; } 
         if(v_in > 463) {out_step=22; }
         if(v_in > 472) {out_step=21; }
         if(v_in > 480) {out_step=20; }
         if(v_in > 489) {out_step=19; }
         if(v_in > 498) {out_step=18; }
         if(v_in > 507) {out_step=17; }
         if(v_in > 516) {out_step=16; }
         if(v_in > 525) {out_step=15; }
         if(v_in > 535) {out_step=14; }
         if(v_in > 544) {out_step=13; }
         if(v_in > 554) {out_step=12; }
         if(v_in > 564) {out_step=11; }
         if(v_in > 575) {out_step=10; }
         if(v_in > 585) {out_step=9;  } 
         if(v_in > 596) {out_step=8;  }
         if(v_in > 606) {out_step=7;  }
         if(v_in > 617) {out_step=6;  }
         if(v_in > 629) {out_step=5;  }
         if(v_in > 640) {out_step=4;  }
         if(v_in > 652) {out_step=3;  }
         if(v_in > 663) {out_step=2;  }
         if(v_in > 676) {out_step=1;  }
         if(v_in > 688) {out_step=0;  }         
 
         if(i_out > 10)  {out_step++;}  //out_step (steps) increment by 1
         if(i_out > 50)  {out_step++;}  //when load current increases
         if(i_out > 100) {out_step++;}
         if(i_out > 150) {out_step++;}
         if(i_out > 200) {out_step++;}
         if(i_out > 250) {out_step++;}
         if(i_out > 310) {out_step++;}
         if(i_out > 400) {out_step++;}
         if(i_out > 480) {out_step++;}
         //if(i_out > 540) {out_step++;}
 
         if(v_ac > 460)
         {
            if(k > 0)  {k--;}
         }
         if(v_ac < 425)
         {
            if(k < 3)  {k++;}
         }
         out_step = out_step + k;
      }
    
      if(loop_number>=100)
      {
         loop_number=100;
         a++;
         if(a==1)
         {
            if(v_ac>465)
            {
               if(out_step<1)
                  {
                     out_step=0;
                  }
               else
               {
                  out_step=out_step-1;
               }
            }
            if(v_ac<425)
            {
               out_step=out_step+1;
            }
            if((v_ac>426)&&(v_ac<441))
            {
               b++;
               if(b>=5)
               {
                  b=0;
                  out_step=out_step+1;
               }
            }
            else
            {
               b=0;
            }
            if((v_ac>=449)&&(v_ac<464))
            {
               c++;
               if(c>=5)
               {
                  c=0;
                  if(out_step<1)
                  {
                     out_step=0;
                  }
                  else
                  {
                     out_step=out_step-1;
                  }
               }
            }
            else
            {
               c=0;
            }
         }
         if(a>=2)
         {
            a=0;
         }
      }
      if(out_step > 31)
      {
         out_step=31;
      }
  
      while(input(PIN_C4))    //Syncronization with MSB of counter 
      {                        //that is zero crossing of sine wave
      }
      while(!input(PIN_C4))
      {
      }
      while(input(PIN_C4))
      {
      }
      while(!input(PIN_C4))
      {
      }
      while(input(PIN_C4))
      {
      }
      while(!input(PIN_C4))
      {
      }
      while(input(PIN_C4))
      {
      }
      while(!input(PIN_C4))
      {
      }
      while(input(PIN_C4))
      {
      }
      while(!input(PIN_C4))
      {
      }    
      while(input(PIN_C4))    //Syncronization with MSB of counter 
      {                        //that is zero crossing of sine wave
      }
      while(!input(PIN_C4))
      {
      }
      while(input(PIN_C4))
      {
      }
      while(!input(PIN_C4))
      {
      }
      while(input(PIN_C4))
      {
      }
      while(!input(PIN_C4))
      {
      }
      while(input(PIN_C4))
      {
      }
      while(!input(PIN_C4))
      {
      }
      while(input(PIN_C4))
      {
      }
      while(!input(PIN_C4))
      {
      }
  
      if(ep1==0)            //Disable EEPROM O/P
      {
         output_low(PIN_C0);
      }
      else
      {
         output_high(PIN_C0);   //Enbable EEPROM O/P
      }
      
      if(((count) < (out_step)) ||((count==30) && (out_step==31)))//tracking
      {                                                   // out_step upward
         count++;
      }
      if (((count) > (out_step)) || ((count==1) && (out_step==0)))//tracking 
      {                                                  //out_step downward
         count--;
      }  
      if(decision==0)         //out_step to start
      {
         count=0;          //select voltage step from table
      }
      output_D(count);
   } 
}

