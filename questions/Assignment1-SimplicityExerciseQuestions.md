Please include your answers to the questions below with your submission, entering into the space below each question
See [Mastering Markdown](https://guides.github.com/features/mastering-markdown/) for github markdown formatting if desired.

**1. How much current does a single LED draw when the output drive is set to "Strong" with the original code?**

Ans: 0.51mA

**2. After commenting out the standard output drive and uncommenting "Weak" drive, how much current does a single LED draw?**

Ans: 0.49mA

**3. Is there a meaningful difference in current between the answers for question 1 and 2? Please explain your answer, 
referencing the [Mainboard Schematic](https://www.silabs.com/documents/public/schematic-files/WSTK-Main-BRD4001A-A01-schematic.pdf) and [AEM Accuracy](https://www.silabs.com/documents/login/user-guides/ug279-brd4104a-user-guide.pdf) section of the user's guide where appropriate.**

Ans: NO. There is no difference because when we calculate the current required to drive the LED as per the circuit, the resistance is 3 Kohms due to series resitance placed across and the voltage drop across led is 2V. So total voltage drop is 3.3-2 = 1.3.
     and now current required is obtained by v/r = 1.3/3K = ~400uA. This is below 1mA which both weak and strong can drive. Thus no difference. 


**4. Using the Energy Profiler with "weak" drive LEDs, what is the average current and energy measured with only LED1 turning on in the main loop?**

Ans: 4.70mA


**5. Using the Energy Profiler with "weak" drive LEDs, what is the average current and energy measured with both LED1 and LED0 turning on in the main loop?**

Ans: 5.03mA
