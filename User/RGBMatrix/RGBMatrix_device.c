// 包含 HUB75 驱动相关头文件
#include "RGBMatrix_device.h"

// HUB75 屏幕参数结构体，全局变量
HUB75_port RGB_Matrix;
/**
 * Initialization routine.
 * You might need to enable access to DWT registers on Cortex-M7
 *   DWT->LAR = 0xC5ACCE55
 */
// DWT 初始化，开启微秒级计数器（Cortex-M7 专用）
void DWT_Init(void)
{
	if (!(CoreDebug->DEMCR & CoreDebug_DEMCR_TRCENA_Msk)) { // 检查 DWT 是否已使能
		CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;     // 使能 DWT 访问
		DWT->CYCCNT = 0;                                    // 清零计数器
		DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;                // 启动 DWT 计数
	}
}

/**
 * Delay routine itself.
 * Time is in microseconds (1/1000000th of a second), not to be
 * confused with millisecond (1/1000th).
 *
 * No need to check an overflow. Let it just tick :)
 *
 * @param uint32_t us  Number of microseconds to delay for
 */
// 微秒级延时函数，利用 DWT 计数器
void DWT_Delay(uint32_t us) // microseconds
{
	uint32_t startTick = DWT->CYCCNT,                      // 记录起始计数
			 delayTicks = us * (SystemCoreClock/1000000);  // 计算目标延时计数

	while (DWT->CYCCNT - startTick < delayTicks);          // 等待计数达到目标
}


// HUB75 屏专用延时，微秒级
void HUB75E_DelayUs(int us) {
	DWT_Delay(us);
}



/**
 * HUB75 Initialization.
 * Give the buffer address to the GUI
 *  
 */
uint32_t _PM_timerSave;
uint32_t freq = 0;
// 启动 HUB75 屏刷新定时器
void HUB75_show()
{
	HAL_TIM_Base_Start_IT(&htim1);           // 启动 TIM1 定时器中断
	_PM_timerSave = __HAL_TIM_GET_COUNTER(&htim1); // 记录当前定时器计数值
}

// HUB75 屏参数初始化
void HUB75_Init(uint8_t width,uint8_t address_size,uint8_t bitDepth)
{
	RGB_Matrix.tile = 1; // 屏幕拼接块数，默认 1
	RGB_Matrix.address_size = address_size; // 地址线数量
	RGB_Matrix.column_select =  1 << (RGB_Matrix.address_size - 1); // 当前选中行
	RGB_Matrix.all_width = width; // 屏幕宽度
	RGB_Matrix.height = (2 << RGB_Matrix.address_size) * abs(RGB_Matrix.tile); // 屏幕高度
	RGB_Matrix.timer_Period = HUB75_MIN_PERIOD; // 定时器周期
	RGB_Matrix.bitDepth=bitDepth; // 色彩位深
	RGB_Matrix.plane = RGB_Matrix.bitDepth; // 当前色彩平面
}




// 按位平面扫描输出一行 RGB565 数据到 HUB75
uint16_t initialRedBit = 0x0800 << 1, initialGreenBit = 0x0040 << 1, initialBlueBit = 0x0001 << 1; // 高位掩码
void RGBMatrixWrite_565Data(uint8_t row , uint8_t plane){
	if(plane == 0){
		initialRedBit   = 0x0800 << 1 ;    // 红色高位掩码初始化
		initialGreenBit = 0x0040 << 1 ;    // 绿色高位掩码初始化
		initialBlueBit  = 0x0001 << 1 ;    // 蓝色高位掩码初始化
	}
	uint16_t *upperSrc, *lowerSrc;         // 上半屏/下半屏指针
	int16_t srcIdx;                        // 当前像素索引
	int8_t srcInc;                         // 索引增量，决定扫描方向
	for (int8_t tile = abs(RGB_Matrix.tile) - 1; tile >= 0; tile--) { // 遍历所有拼接块
		if(RGB_Matrix.tile < 0)            // 负数表示倒装，反向扫描
		{
			lowerSrc = RGB_Matrix.BlackImage + (tile * RGB_Matrix.all_width * (RGB_Matrix.height / 2)) + RGB_Matrix.all_width * ((RGB_Matrix.height / 2) - 1 - row); // 下半屏起始地址
			upperSrc = lowerSrc + RGB_Matrix.all_width * (RGB_Matrix.height / 2); // 上半屏起始地址
			srcIdx = RGB_Matrix.all_width - 1; // 从右向左
			srcInc = -1;                   // 索引递减
		}
		else                              // 正常拼接，正向扫描
		{
			upperSrc = RGB_Matrix.BlackImage + (tile * RGB_Matrix.all_width * (RGB_Matrix.height / 2)) + (RGB_Matrix.all_width * row); // 上半屏起始地址
			lowerSrc = upperSrc + RGB_Matrix.all_width * (RGB_Matrix.height / 2); // 下半屏起始地址
			srcIdx = 0;                    // 从左向右
			srcInc = 1;                    // 索引递增
		}
		for(uint16_t x = 0; x < RGB_Matrix.all_width; x++, srcIdx += srcInc) { // 遍历每一列
			// 上半屏红色输出
			if(upperSrc[srcIdx] & initialRedBit)
				R1_GPIO_Port->BSRR = R1_Pin;           // 置位红色引脚
			else
				R1_GPIO_Port->BSRR = (uint32_t)R1_Pin << 16u; // 复位红色引脚
			// 上半屏绿色输出
			if(upperSrc[srcIdx] & initialGreenBit)
				G1_GPIO_Port->BSRR = G1_Pin;           // 置位绿色引脚
			else
				G1_GPIO_Port->BSRR = (uint32_t)G1_Pin << 16u; // 复位绿色引脚
			// 上半屏蓝色输出
			if(upperSrc[srcIdx] & initialBlueBit)
				B1_GPIO_Port->BSRR = B1_Pin;           // 置位蓝色引脚
			else
				B1_GPIO_Port->BSRR = (uint32_t)B1_Pin << 16u; // 复位蓝色引脚
			// 下半屏红色输出
			if(lowerSrc[srcIdx] & initialRedBit)
				R2_GPIO_Port->BSRR = R2_Pin;           // 置位红色引脚
			else
				R2_GPIO_Port->BSRR = (uint32_t)R2_Pin << 16u; // 复位红色引脚
			// 下半屏绿色输出
			if(lowerSrc[srcIdx] & initialGreenBit)
				G2_GPIO_Port->BSRR = G2_Pin;           // 置位绿色引脚
			else
				G2_GPIO_Port->BSRR = (uint32_t)G2_Pin << 16u; // 复位绿色引脚
			// 下半屏蓝色输出
			if(lowerSrc[srcIdx] & initialBlueBit)
				B2_GPIO_Port->BSRR = B2_Pin;           // 置位蓝色引脚
			else
				B2_GPIO_Port->BSRR = (uint32_t)B2_Pin << 16u; // 复位蓝色引脚
			CLK_GPIO_Port->BSRR = (uint32_t)CLK_Pin << 16u ;  // 时钟信号拉低
			CLK_GPIO_Port->BSRR = CLK_Pin;                    // 时钟信号拉高
		} //end x
	}// end tile
	initialRedBit <<= 1;      // 红色掩码左移，准备下一个平面
	initialGreenBit <<= 1;    // 绿色掩码左移
	initialBlueBit <<= 1;     // 蓝色掩码左移
}

//timer interrupt callback
// TIM1 定时器溢出中断回调，驱动 HUB75 刷新
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Instance == htim1.Instance) // 判断是否为 TIM1 中断
	{
		OE_GPIO_Port->BSRR = OE_Pin; // 使能输出
		LAT_GPIO_Port->BSRR = (uint32_t)LAT_Pin << 16u; // 复位锁存
		LAT_GPIO_Port->BSRR = LAT_Pin; // 置位锁存，锁存数据
		HAL_TIM_Base_Stop_IT(&htim1); // 停止定时器，防止重入
		//uint8_t prevPlane = plane; // 可选调试
		//printf("prevPlane:%d\n",prevPlane);
		LAT_GPIO_Port->BSRR = (uint32_t)LAT_Pin << 16u; // 再次复位锁存
		if(RGB_Matrix.plane == 0) // 只有第 0 平面时切换行地址
		{
			if(RGB_Matrix.column_select & 0x1)
				A_GPIO_Port->BSRR = A_Pin; // 行地址 A 置位
			else
				A_GPIO_Port->BSRR = (uint32_t)A_Pin << 16u; // 行地址 A 复位
			if(RGB_Matrix.column_select & 0x2)
				B_GPIO_Port->BSRR = B_Pin; // 行地址 B 置位
			else
				B_GPIO_Port->BSRR = (uint32_t)B_Pin << 16u; // 行地址 B 复位
			if(RGB_Matrix.column_select & 0x4)
				C_GPIO_Port->BSRR = C_Pin; // 行地址 C 置位
			else
				C_GPIO_Port->BSRR = (uint32_t)C_Pin << 16u; // 行地址 C 复位
			if(RGB_Matrix.column_select & 0x8)
				D_GPIO_Port->BSRR = D_Pin; // 行地址 D 置位
			else
				D_GPIO_Port->BSRR = (uint32_t)D_Pin << 16u; // 行地址 D 复位
			if(RGB_Matrix.column_select & 0x10)
				E_GPIO_Port->BSRR = E_Pin; // 行地址 E 置位
			else
				E_GPIO_Port->BSRR = (uint32_t)E_Pin << 16u; // 行地址 E 复位
		}
		if(++RGB_Matrix.plane >= RGB_Matrix.bitDepth) // 平面计数递增，超过最大位深归零
		{
			RGB_Matrix.plane = 0;
			if(++RGB_Matrix.column_select >= (1 << RGB_Matrix.address_size)) // 行地址递增，超过最大归零
			{
				RGB_Matrix.column_select = 0;               // 行地址归零
			}       
		}  
		__HAL_TIM_SET_COUNTER(&htim1,0); // 定时器计数器清零
		htim1.Instance->ARR = RGB_Matrix.timer_Period << RGB_Matrix.plane; // 设置下次中断周期
		HAL_TIM_Base_Start_IT(&htim1); // 重新启动定时器
		OE_GPIO_Port->BSRR = (uint32_t)OE_Pin << 16u;; // 关闭输出使能
		RGBMatrixWrite_565Data(RGB_Matrix.column_select,RGB_Matrix.plane); // 输出当前行数据
		if(RGB_Matrix.plane == 0) // 只有第 0 平面时更新周期
		{
			uint32_t elapsed = __HAL_TIM_GET_COUNTER(&htim1) - _PM_timerSave; // 计算本周期消耗时间
			RGB_Matrix.timer_Period = ((RGB_Matrix.timer_Period * 7) + elapsed) / 8; // 周期自适应平滑滤波
			if (RGB_Matrix.timer_Period < HUB75_MIN_PERIOD) RGB_Matrix.timer_Period = HUB75_MIN_PERIOD; // 限制最小周期
		}
	}
}

// 生成彩虹色 RGB565 数据，供测试用
uint16_t Wheel(uint8_t WheelPos) {
	if(WheelPos < 16) {
		return ((32 - WheelPos) << 11) |(WheelPos << 5) | 0; // 红到绿渐变
	} else if(WheelPos < 32) {
		WheelPos -= 16;
		return 0 | ((32 - WheelPos) << 5) | WheelPos;         // 绿到蓝渐变
	} else {
		WheelPos -= 32;
		return (WheelPos <<11 )| 0 | (32 - WheelPos);         // 蓝到红渐变
	}
}


